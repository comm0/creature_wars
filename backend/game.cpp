#include "game.h"

#ifndef NDEBUG
#include "debug_console.h"
#endif
#include "game_constants.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <utility>

namespace
{
constexpr int start_base_edge_gap = 3;
constexpr int max_spawn_queue = 9;
constexpr int wildlife_minimum_base_distance = 6;
constexpr int wildlife_maximum_base_distance = 10;
constexpr std::chrono::seconds corpse_lifetime{30};
const std::string spawn_prefix = "spawn:";
const std::string training_prefix = "training:";
const std::string research_prefix = "research:";
const std::string upgrade_key = "upgrade";

bool starts_with(const std::string& text_p, const std::string& prefix_p)
{
    return text_p.rfind(prefix_p, 0) == 0;
}
constexpr std::size_t match_base_count = 3;

std::array<position_t, match_base_count> starting_base_positions(
    int edge_offset_p
)
{
    const auto top_row = edge_offset_p;
    const auto bottom_row = game_constants::map_row_count - 1 - edge_offset_p;
    const auto left_column = 2 * edge_offset_p;
    const auto right_column = game_constants::map_column_count - 1 - left_column;
    const auto middle_column = (left_column + right_column) / 2;

    return {
        position_t{middle_column, bottom_row},
        position_t{left_column, top_row},
        position_t{right_column, top_row}
    };
}

int position_distance(position_t left_p, position_t right_p) noexcept
{
    return std::max(
        std::abs(left_p.column_ - right_p.column_),
        std::abs(left_p.row_ - right_p.row_)
    );
}

bool has_free_adjacent_position(
    const game_map_t& game_map_p,
    position_t position_p
)
{
    for (auto row = position_p.row_ - 1; row <= position_p.row_ + 1; ++row) {
        for (
            auto column = position_p.column_ - 1;
            column <= position_p.column_ + 1;
            ++column
        ) {
            if (column == position_p.column_ && row == position_p.row_) {
                continue;
            }

            if (game_map_p.can_place_creature({column, row})) {
                return true;
            }
        }
    }

    return false;
}

struct reward_share_t
{
    std::string group_;
    int amount_;
    int remainder_;
};

std::vector<reward_share_t> split_reward(
    int amount_p,
    const std::unordered_map<std::string, int>& damage_by_group_p
)
{
    std::vector<reward_share_t> shares;
    auto total_damage = 0;

    for (const auto& [group, damage] : damage_by_group_p) {
        total_damage += damage;
    }

    if (amount_p <= 0 || total_damage <= 0) {
        return shares;
    }

    auto assigned = 0;

    for (const auto& [group, damage] : damage_by_group_p) {
        const auto weighted_reward = amount_p * damage;
        const auto share = weighted_reward / total_damage;
        shares.push_back({group, share, weighted_reward % total_damage});
        assigned += share;
    }

    std::sort(
        shares.begin(),
        shares.end(),
        [](const auto& left_p, const auto& right_p) {
            return left_p.remainder_ != right_p.remainder_
                ? left_p.remainder_ > right_p.remainder_
                : left_p.group_ < right_p.group_;
        }
    );

    for (auto index = 0; index < amount_p - assigned; ++index) {
        ++shares[static_cast<std::size_t>(index)].amount_;
    }

    return shares;
}
}

/*! 
    \class game_t
    \inmodule CreatureWars
    \brief Owns the game thread and serializes all game actions.
*/

/*! Creates a stopped game. */
game_t::game_t(
    const std::string& creature_types_json_p,
    const std::string& base_types_json_p,
    const std::string& economy_json_p,
    const std::string& tech_tree_json_p,
    const std::string& ai_profiles_json_p
)
    : wildlife_spawner_(
        scheduler_,
        {
            [this](std::string_view identifier_p, std::string_view group_p) {
                spawn_creature_near_group(identifier_p, group_p);
            },
            [this](std::string_view group_p) {
                return spawn_lair_near_group(group_p);
            },
            [this](std::uint64_t lair_id_p) {
                return spawn_troll_near_lair(lair_id_p);
            },
            [this](std::uint64_t lair_id_p) {
                return lair_exists(lair_id_p);
            }
        }
    )
    , creature_type_registry_(creature_types_json_p)
    , base_type_registry_(base_types_json_p)
    , economy_(parse_economy_settings(economy_json_p))
    , tech_tree_(tech_tree_json_p)
    , ai_profiles_(ai_profiles_json_p)
{
    for (const auto& base_type : base_type_registry_.all()) {
        for (const auto& unit : base_type.units()) {
            static_cast<void>(creature_type_registry_.get(unit.creature_));
        }
    }
}

game_t::~game_t()
{
    stop();
}

/*! Starts the game thread and reports state changes to \a observer_p. */
void game_t::start(igame_observer_t& observer_p)
{
    {
        std::lock_guard<std::mutex> lock(actions_mutex_);

        if (thread_running_) {
            throw std::logic_error("Game is already running.");
        }

        observer_ = &observer_p;
        thread_running_ = true;
    }

    game_clock_.resume();

    try {
        thread_ = std::jthread([this](const std::stop_token& stop_token_p) {
            run(stop_token_p);
        });
    } catch (...) {
        game_clock_.pause();
        std::lock_guard<std::mutex> lock(actions_mutex_);
        observer_ = nullptr;
        thread_running_ = false;
        throw;
    }
}

/*! Stops the game thread after its current action. */
void game_t::stop()
{
    {
        std::lock_guard<std::mutex> lock(actions_mutex_);

        if (!thread_running_) {
            return;
        }

        thread_running_ = false;
    }

    thread_.request_stop();
    actions_available_.notify_one();
    thread_.join();
    game_clock_.pause();

    {
        std::lock_guard<std::mutex> lock(actions_mutex_);
        actions_.clear();
        observer_ = nullptr;
    }
}

/*! Queues \a event_p for the game thread. */
void game_t::post(std::function<void()> event_p)
{
    if (!event_p) {
        throw std::invalid_argument("Game event must be callable.");
    }

    {
        std::lock_guard<std::mutex> lock(actions_mutex_);

        if (!thread_running_) {
            throw std::logic_error("Game is not running.");
        }

        actions_.push_back(std::move(event_p));
    }

    actions_available_.notify_one();
}

void game_t::run(const std::stop_token& stop_token_p)
{
    if (!tick_scheduled_) {
        tick_scheduled_ = true;
        schedule_tick(game_clock_.now() + game_constants::tick_interval);
    }

    publish_creatures();

    while (!stop_token_p.stop_requested()) {
        auto next_wake_time = scheduler_.next_time();
        const auto next_movement_time = creatures_.next_movement_time();

        if (next_movement_time.has_value()
            && (!next_wake_time.has_value() || *next_movement_time < *next_wake_time)) {
            next_wake_time = next_movement_time;
        }

        {
            std::unique_lock lock(actions_mutex_);
            const auto wake_condition = [this, &stop_token_p]() {
                return stop_token_p.stop_requested() || !actions_.empty();
            };

            if (next_wake_time.has_value()) {
                actions_available_.wait_until(
                    lock,
                    game_clock_.wall_time_for(*next_wake_time),
                    wake_condition
                );
            } else {
                actions_available_.wait(lock, wake_condition);
            }
        }

        if (stop_token_p.stop_requested()) {
            break;
        }

        collect_actions();

        for (auto& event : scheduler_.take_due(game_clock_.now())) {
            dispatcher_.enqueue(std::move(event));
        }

        dispatcher_.dispatch_pending();

        const auto movement_time = game_clock_.now();
        const auto next_due_movement = creatures_.next_movement_time();

        if (next_due_movement.has_value()
            && *next_due_movement <= movement_time) {
            dispatcher_.enqueue([this, movement_time]() {
                dispatch_movement(movement_time);
            });
            dispatcher_.dispatch_pending();
        }
    }
}

void game_t::collect_actions()
{
    std::deque<std::function<void()>> collected_actions;

    {
        std::lock_guard<std::mutex> lock(actions_mutex_);
        collected_actions.swap(actions_);
    }

    while (!collected_actions.empty()) {
        dispatcher_.enqueue(std::move(collected_actions.front()));
        collected_actions.pop_front();
    }
}

void game_t::request_spawn_creature(
    std::string identifier_p,
    position_t position_p
)
{
    post([this, identifier = std::move(identifier_p), position_p]() mutable {
        spawn_creature(std::move(identifier), position_p);
    });
}

void game_t::request_spawn_base(std::string identifier_p, position_t center_p)
{
    post([this, identifier = std::move(identifier_p), center_p]() mutable {
        spawn_base(std::move(identifier), center_p);
    });
}

void game_t::request_order_base_action(std::string key_p)
{
    post([this, key = std::move(key_p)]() {
        auto* controller = player_controller();
        if (controller != nullptr) {
            order_base_action(*controller, key);
        }
    });
}

void game_t::request_start_match(
    std::string player_base_identifier_p,
    std::unordered_map<std::string, ai_difficulty_t> ai_difficulties_p
)
{
    post([
        this,
        identifier = std::move(player_base_identifier_p),
        difficulties = std::move(ai_difficulties_p)
    ]() {
        start_match(identifier, difficulties);
    });
}

void game_t::start_match(
    const std::string& player_base_identifier_p,
    const std::unordered_map<std::string, ai_difficulty_t>& ai_difficulties_p
)
{
    for (auto& controller : controllers_) {
        controller->orders_->clear();
        retired_controllers_.push_back(std::move(controller));
    }
    controllers_.clear();
    clear_world();

    const auto& player_base = base_type_registry_.get(player_base_identifier_p);

    if (!player_base.is_match_base()) {
        throw std::invalid_argument("Player base must be a match base.");
    }

    const auto now = game_clock_.now();
    ++match_id_;
    match_active_ = true;
    match_started_at_ = now;
    std::vector<const base_type_t*> enemy_bases;
    earned_resources_.clear();

    for (const auto& base_type : base_type_registry_.all()) {
        if (!base_type.is_match_base()) {
            continue;
        }

        earned_resources_.emplace(base_type.group(), resource_reward_t{});

        if (&base_type != &player_base) {
            enemy_bases.push_back(&base_type);
        }
    }

    if (enemy_bases.size() + 1 != match_base_count) {
        throw std::logic_error("A match requires exactly three bases.");
    }

    auto largest_base_size = player_base.size();

    for (const auto* enemy_base : enemy_bases) {
        largest_base_size = std::max(largest_base_size, enemy_base->size());
    }

    const auto edge_offset = start_base_edge_gap + largest_base_size / 2;
    const auto starting_positions = starting_base_positions(edge_offset);

    const auto* spawned_player_base = spawn_base(
        player_base.identifier(),
        starting_positions[0]
    );

    if (spawned_player_base == nullptr) {
        throw std::logic_error("Could not spawn the player base.");
    }

    auto& player = create_controller(
        *spawned_player_base,
        true,
        ai_difficulty_t::normal
    );
    schedule_income(
        player.state_.base_id_,
        &player_state_t::gold_,
        now + player.state_.gold_.income_interval_
    );
    schedule_income(
        player.state_.base_id_,
        &player_state_t::food_,
        now + player.state_.food_.income_interval_
    );

    for (std::size_t index = 0; index < enemy_bases.size(); ++index) {
        const auto& enemy_base = *enemy_bases[index];
        const auto* spawned_enemy_base = spawn_base(
            enemy_base.identifier(),
            starting_positions[index + 1]
        );

        if (spawned_enemy_base == nullptr) {
            throw std::logic_error("Could not spawn an AI base.");
        }

        const auto difficulty = ai_difficulties_p.contains(enemy_base.identifier())
            ? ai_difficulties_p.at(enemy_base.identifier())
            : ai_difficulty_t::normal;
        auto& controller = create_controller(
            *spawned_enemy_base,
            false,
            difficulty
        );
        schedule_income(
            controller.state_.base_id_,
            &player_state_t::gold_,
            now + controller.state_.gold_.income_interval_
        );
        schedule_income(
            controller.state_.base_id_,
            &player_state_t::food_,
            now + controller.state_.food_.income_interval_
        );
        schedule_ai_decision(
            controller.state_.base_id_,
            now + controller.profile_->decision_interval_
        );
    }

    publish_controller(player);
    publish_base_actions();
    wildlife_spawner_.start(now);
    dispatcher_.enqueue([this]() {
        retired_controllers_.clear();
    });
}

base_controller_t& game_t::create_controller(
    const base_t& base_p,
    bool human_controlled_p,
    ai_difficulty_t difficulty_p
)
{
    auto controller = std::make_unique<base_controller_t>();
    controller->state_ = {
        base_p.id(),
        base_p.type().group(),
        base_p.type().name(),
        base_p.level(),
        {
            economy_.starting_gold_,
            economy_.gold_income_.amount_,
            economy_.gold_income_.interval_,
            0
        },
        {
            economy_.starting_food_,
            economy_.food_income_.amount_,
            economy_.food_income_.interval_,
            0
        }
    };
    controller->human_controlled_ = human_controlled_p;
    controller->difficulty_ = difficulty_p;
    controller->profile_ = &ai_profiles_.profile(difficulty_p);
    auto* result = controller.get();
    const auto base_id = controller->state_.base_id_;
    controller->orders_ = std::make_unique<base_orders_t>(
        scheduler_,
        [this, base_id](const std::string& key_p) {
            auto* current = controller_for_base(base_id);
            return current == nullptr || complete_base_order(*current, key_p);
        },
        [this, base_id]() {
            auto* current = controller_for_base(base_id);
            if (current == nullptr) {
                return;
            }

            publish_controller(*current);
            if (current->human_controlled_) {
                publish_base_actions();
            }
        }
    );
    controllers_.push_back(std::move(controller));
    publish_controller(*result);
    return *result;
}

void game_t::clear_world()
{
    wildlife_spawner_.reset();

    for (const auto id : corpse_ids_) {
        observer_->on_corpse_removed(id);
    }

    corpse_ids_.clear();
    std::vector<std::uint64_t> creature_ids;
    creatures_.for_each([&creature_ids](const creature_t& creature_p) {
        creature_ids.push_back(creature_p.id());
    });

    for (const auto id : creature_ids) {
        auto* creature = creatures_.find(id);
        game_map_.remove_creature(*creature);
        visibility_system_.remove_creature(*creature, creatures_);
        observer_->on_creature_removed(id);
        creatures_.remove(id);
    }

    dead_creature_ids_.clear();

    for (const auto& base : bases_) {
        game_map_.remove_base(*base);
        observer_->on_base_removed(base->id());
    }

    bases_.clear();
    earned_resources_.clear();
}

base_t* game_t::spawn_base(const std::string& identifier_p, position_t center_p)
{
    const auto& base_type = base_type_registry_.get(identifier_p);
    const auto group_has_base = base_type.is_match_base() && std::any_of(
        bases_.begin(),
        bases_.end(),
        [&base_type](const auto& base_p) {
            return base_p->type().group() == base_type.group();
        }
    );

    if (group_has_base) {
        return nullptr;
    }

    const auto half_size = base_type.size() / 2;
    const position_t position{
        center_p.column_ - half_size,
        center_p.row_ - half_size
    };

    if (!game_map_.can_place_base(position, base_type.size())) {
        return nullptr;
    }

    auto& base = *bases_.emplace_back(std::make_unique<base_t>(
        creatures_t::allocate_id(),
        base_type,
        position
    ));
    game_map_.place_base(base);
    observer_->on_base_created(base);
    return &base;
}

std::vector<base_action_state_t> game_t::base_actions(
    const base_controller_t& controller_p
) const
{
    std::vector<base_action_state_t> actions;

    if (!match_active_) {
        return actions;
    }

    const auto* base = find_base(controller_p.state_.base_id_);

    if (base == nullptr || base->is_dead()) {
        return actions;
    }

    const auto now = game_clock_.now();
    const auto add_action = [&controller_p, &actions, now](
        std::string key_p,
        base_action_kind_t kind_p,
        std::string name_p,
        std::string subject_p,
        std::uint32_t color_p,
        int level_p,
        const timed_cost_t& cost_p
    ) {
        const auto progress = controller_p.orders_->progress(key_p);
        auto remaining = std::chrono::milliseconds{0};

        if (progress.finish_time_.has_value()) {
            remaining = std::max(
                std::chrono::milliseconds{0},
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    *progress.finish_time_ - now
                )
            );
        }

        actions.push_back({
            std::move(key_p),
            kind_p,
            std::move(name_p),
            std::move(subject_p),
            color_p,
            level_p,
            cost_p,
            progress.queued_,
            remaining
        });
    };

    for (const auto& unit : base->type().units()) {
        if (unit.tier_ > base->level()) {
            continue;
        }

        const auto& creature = creature_type_registry_.get(unit.creature_);

        if (unit.training_.has_value()
            && !controller_p.trained_units_.contains(unit.creature_)) {
            add_action(
                training_prefix + unit.creature_,
                base_action_kind_t::training,
                creature.name() + " Training",
                unit.creature_,
                creature.color(),
                unit.tier_,
                *unit.training_
            );
        } else {
            add_action(
                spawn_prefix + unit.creature_,
                base_action_kind_t::spawn,
                creature.name(),
                unit.creature_,
                creature.color(),
                unit.tier_,
                unit.spawn_
            );
        }
    }

    for (const auto& research : tech_tree_.researches()) {
        const auto completed = static_cast<std::size_t>(research_level(
            controller_p,
            research.identifier_
        ));

        if (completed >= research.levels_.size()
            || research.levels_[completed].base_level_ > base->level()) {
            continue;
        }

        add_action(
            research_prefix + research.identifier_,
            base_action_kind_t::research,
            research.name_,
            research.identifier_,
            base->type().color(),
            static_cast<int>(completed) + 1,
            research.levels_[completed].cost_
        );
    }

    const auto* upgrade = tech_tree_.find_upgrade(base->level() + 1);

    if (upgrade != nullptr && base->level() < base->type().max_level()) {
        add_action(
            upgrade_key,
            base_action_kind_t::upgrade,
            "Upgrade Base",
            base->type().identifier(),
            base->type().color(),
            upgrade->level_,
            upgrade->cost_
        );
    }

    return actions;
}

bool game_t::order_base_action(
    base_controller_t& controller_p,
    const std::string& key_p
)
{
    if (!match_active_) {
        return false;
    }

    const auto actions = base_actions(controller_p);
    const auto action = std::find_if(
        actions.begin(),
        actions.end(),
        [&key_p](const auto& action_p) {
            return action_p.key_ == key_p;
        }
    );

    if (action == actions.end()) {
        return false;
    }

    const auto queue_limit = action->kind_ == base_action_kind_t::spawn
        ? max_spawn_queue
        : 1;
    auto& gold = controller_p.state_.gold_.amount_;
    auto& food = controller_p.state_.food_.amount_;

    if (action->queued_ >= queue_limit
        || gold < action->cost_.cost_.gold_
        || food < action->cost_.cost_.food_) {
        return false;
    }

    gold -= action->cost_.cost_.gold_;
    food -= action->cost_.cost_.food_;
    publish_controller(controller_p);
    controller_p.orders_->enqueue(key_p, action->cost_.duration_, game_clock_.now());
    return true;
}

bool game_t::complete_base_order(
    base_controller_t& controller_p,
    const std::string& key_p
)
{
    if (!match_active_) {
        return true;
    }

    auto* base = find_base(controller_p.state_.base_id_);

    if (base == nullptr || base->is_dead()) {
        return true;
    }

    if (key_p == upgrade_key) {
        base->set_level(base->level() + 1);
        controller_p.state_.base_level_ = base->level();
        apply_base_stats(controller_p, *base);
        update_income_intervals(controller_p);
        publish_controller(controller_p);
        return true;
    }

    if (starts_with(key_p, spawn_prefix)) {
        const auto position = game_map_.free_position_around(*base);

        if (!position.has_value()) {
            return false;
        }

        spawn_creature(key_p.substr(spawn_prefix.size()), *position);
        return true;
    }

    if (starts_with(key_p, training_prefix)) {
        controller_p.trained_units_.insert(key_p.substr(training_prefix.size()));
        return true;
    }

    if (starts_with(key_p, research_prefix)) {
        ++controller_p.research_levels_[key_p.substr(research_prefix.size())];
        apply_base_stats(controller_p, *base);
    }

    return true;
}

void game_t::publish_base_actions()
{
    const auto* controller = player_controller();
    if (controller != nullptr) {
        observer_->on_base_actions_changed(
            controller->state_.base_id_,
            base_actions(*controller)
        );
    }
}

void game_t::publish_controller(const base_controller_t& controller_p)
{
    observer_->on_base_controller_changed({
        controller_p.state_,
        controller_p.human_controlled_,
        to_string(controller_p.difficulty_),
        to_string(controller_p.strategy_)
    });

    if (controller_p.human_controlled_) {
        observer_->on_player_state_changed(controller_p.state_);
    }
}

int game_t::research_level(
    const base_controller_t& controller_p,
    const std::string& identifier_p
) const
{
    const auto level = controller_p.research_levels_.find(identifier_p);
    return level == controller_p.research_levels_.end() ? 0 : level->second;
}

int game_t::research_value(
    const base_controller_t& controller_p,
    research_effect_t effect_p
) const
{
    auto value = 0;

    for (const auto& research : tech_tree_.researches()) {
        const auto level = research_level(controller_p, research.identifier_);

        if (research.effect_ == effect_p && level > 0) {
            value += research.levels_[static_cast<std::size_t>(level - 1)].value_;
        }
    }

    return value;
}

void game_t::apply_base_stats(
    base_controller_t& controller_p,
    base_t& base_p
)
{
    const auto& level = base_p.type().level_stats(base_p.level());
    const auto value = [this, &controller_p](research_effect_t effect_p) {
        return research_value(controller_p, effect_p);
    };

    base_p.apply_stats({
        level.health_ * (100 + value(research_effect_t::base_health_percent)) / 100,
        level.attack_ * (100 + value(research_effect_t::base_damage_percent)) / 100,
        base_p.type().range() + value(research_effect_t::base_range_bonus),
        std::max(1, value(research_effect_t::shot_count)),
        value(research_effect_t::base_regeneration),
        value(research_effect_t::healing_aura)
    });
    observer_->on_base_changed(base_p);
}

void game_t::update_income_intervals(base_controller_t& controller_p)
{
    const auto speed_percent = 100
        + economy_.level_income_bonus_percent_ * (controller_p.state_.base_level_ - 1);
    const auto scaled = [speed_percent](std::chrono::milliseconds interval_p) {
        return std::chrono::milliseconds{interval_p.count() * 100 / speed_percent};
    };

    controller_p.state_.gold_.income_interval_ = scaled(economy_.gold_income_.interval_);
    controller_p.state_.food_.income_interval_ = scaled(economy_.food_income_.interval_);
}

void game_t::regenerate_bases()
{
    for (const auto& base : bases_) {
        if (base->regenerate()) {
            observer_->on_base_health_changed(base->id(), base->health());
        }
    }
}

void game_t::heal_near_bases()
{
    for (const auto& base : bases_) {
        if (base->is_dead() || base->stats().healing_aura_ <= 0) {
            continue;
        }

        std::vector<std::uint64_t> creature_ids;
        creatures_.for_each([&base, &creature_ids](const creature_t& creature_p) {
            if (creature_p.type().group() == base->type().group()
                && base->distance_to(creature_p.position()) <= base->stats().range_) {
                creature_ids.push_back(creature_p.id());
            }
        });

        for (const auto id : creature_ids) {
            auto* creature = creatures_.find(id);

            if (creature != nullptr && creature->heal(base->stats().healing_aura_)) {
                observer_->on_creature_health_changed(id, creature->health());
            }
        }
    }
}

void game_t::damage_area(
    const creature_t& attacker_p,
    position_t center_p,
    int radius_p
)
{
    const auto& group = attacker_p.type().group();
    std::vector<std::uint64_t> target_ids;

    creatures_.for_each([&](const creature_t& creature_p) {
        if (!creature_p.is_dead()
            && creature_p.type().group() != group
            && position_distance(creature_p.position(), center_p) <= radius_p) {
            target_ids.push_back(creature_p.id());
        }
    });

    for (const auto& base : bases_) {
        if (!base->is_dead()
            && base->type().group() != group
            && base->distance_to(center_p) <= radius_p) {
            target_ids.push_back(base->id());
        }
    }

    const auto damage = attacker_p.type().attack();

    for (const auto id : target_ids) {
        damage_target(id, damage, group);
    }

    observer_->on_area_attack(center_p, radius_p, attacker_p.type().area_color());
}

base_t* game_t::find_base(std::uint64_t id_p) noexcept
{
    const auto base = std::find_if(
        bases_.begin(),
        bases_.end(),
        [id_p](const auto& base_p) {
            return base_p->id() == id_p;
        }
    );

    return base == bases_.end() ? nullptr : base->get();
}

const base_t* game_t::find_base(std::uint64_t id_p) const noexcept
{
    const auto base = std::find_if(
        bases_.begin(),
        bases_.end(),
        [id_p](const auto& base_p) {
            return base_p->id() == id_p;
        }
    );

    return base == bases_.end() ? nullptr : base->get();
}

base_controller_t* game_t::controller_for_base(std::uint64_t base_id_p) noexcept
{
    const auto controller = std::find_if(
        controllers_.begin(),
        controllers_.end(),
        [base_id_p](const auto& controller_p) {
            return controller_p->state_.base_id_ == base_id_p;
        }
    );
    return controller == controllers_.end() ? nullptr : controller->get();
}

const base_controller_t* game_t::controller_for_base(
    std::uint64_t base_id_p
) const noexcept
{
    const auto controller = std::find_if(
        controllers_.begin(),
        controllers_.end(),
        [base_id_p](const auto& controller_p) {
            return controller_p->state_.base_id_ == base_id_p;
        }
    );
    return controller == controllers_.end() ? nullptr : controller->get();
}

base_controller_t* game_t::controller_for_group(std::string_view group_p) noexcept
{
    const auto controller = std::find_if(
        controllers_.begin(),
        controllers_.end(),
        [group_p](const auto& controller_p) {
            return controller_p->state_.base_group_ == group_p;
        }
    );
    return controller == controllers_.end() ? nullptr : controller->get();
}

base_controller_t* game_t::player_controller() noexcept
{
    const auto controller = std::find_if(
        controllers_.begin(),
        controllers_.end(),
        [](const auto& controller_p) {
            return controller_p->human_controlled_;
        }
    );
    return controller == controllers_.end() ? nullptr : controller->get();
}

const base_controller_t* game_t::player_controller() const noexcept
{
    const auto controller = std::find_if(
        controllers_.begin(),
        controllers_.end(),
        [](const auto& controller_p) {
            return controller_p->human_controlled_;
        }
    );
    return controller == controllers_.end() ? nullptr : controller->get();
}

const base_t* game_t::find_match_base(std::string_view group_p) const noexcept
{
    const auto base = std::find_if(
        bases_.begin(),
        bases_.end(),
        [group_p](const auto& base_p) {
            return !base_p->is_dead()
                && base_p->type().is_match_base()
                && base_p->type().group() == group_p;
        }
    );

    return base == bases_.end() ? nullptr : base->get();
}

/*! Creates a creature of the requested type at a map position. */
creature_t* game_t::spawn_creature(
    std::string identifier_p,
    position_t position_p
)
{
    if (!game_map_.can_place_creature(position_p)) {
        return nullptr;
    }

    const auto& creature_type = creature_type_registry_.get(identifier_p);
    auto& creature = creatures_.create(creature_type, position_p);

    if (!game_map_.place_creature(creature, position_p)) {
        throw std::logic_error("Could not place a creature at the requested position.");
    }

    observer_->on_creature_created(creature);
    visibility_system_.add_creature(
        creature,
        game_map_,
        creatures_,
        [this](std::uint64_t observer_id_p, std::uint64_t spotted_id_p) {
            notify_creature_spotted(observer_id_p, spotted_id_p);
        }
    );

    return &creature;
}

void game_t::spawn_creature_near_group(
    std::string_view identifier_p,
    std::string_view group_p
)
{
    const auto* base = find_match_base(group_p);

    if (base == nullptr) {
        return;
    }

    const auto& creature_type = creature_type_registry_.get(identifier_p);
    const auto position = game_map_.random_position_near(
        *base,
        wildlife_minimum_base_distance,
        wildlife_maximum_base_distance,
        [this, &creature_type](position_t position_p) {
            return game_map_.can_place_creature(position_p)
                && !visibility_system_.is_visible_to_enemy(
                    position_p,
                    creature_type.group(),
                    creatures_
                );
        }
    );

    if (position.has_value()) {
        spawn_creature(std::string(identifier_p), *position);
    }
}

std::optional<std::uint64_t> game_t::spawn_lair_near_group(
    std::string_view group_p
)
{
    const auto* base = find_match_base(group_p);

    if (base == nullptr) {
        return std::nullopt;
    }

    const auto position = game_map_.random_position_near(
        *base,
        wildlife_minimum_base_distance,
        wildlife_maximum_base_distance,
        [this](position_t position_p) {
            return game_map_.can_place_base(position_p, 1)
                && has_free_adjacent_position(game_map_, position_p);
        }
    );

    if (!position.has_value()) {
        return std::nullopt;
    }

    auto* lair = spawn_base("troll_lair", *position);
    return lair == nullptr
        ? std::nullopt
        : std::optional<std::uint64_t>(lair->id());
}

bool game_t::spawn_troll_near_lair(std::uint64_t lair_id_p)
{
    const auto* lair = find_base(lair_id_p);

    if (lair == nullptr
        || lair->is_dead()
        || lair->type().identifier() != "troll_lair") {
        return false;
    }

    const auto position = game_map_.free_position_around(*lair);
    return position.has_value()
        && spawn_creature("troll", *position) != nullptr;
}

bool game_t::lair_exists(std::uint64_t lair_id_p) const noexcept
{
    const auto* lair = find_base(lair_id_p);
    return lair != nullptr
        && !lair->is_dead()
        && lair->type().identifier() == "troll_lair";
}

void game_t::schedule_tick(game_scheduler_t::time_point_t time_p)
{
    scheduler_.schedule(time_p, [this](game_scheduler_t::time_point_t scheduled_time_p) {
        dispatch_tick();
        schedule_tick(std::max(
            scheduled_time_p + game_constants::tick_interval,
            game_clock_.now()
        ));
    });
}

void game_t::schedule_income(
    std::uint64_t base_id_p,
    resource_state_t player_state_t::* resource_p,
    game_scheduler_t::time_point_t time_p
)
{
    scheduler_.schedule(
        time_p,
        [this, base_id_p, resource_p, match_id = match_id_](
            game_scheduler_t::time_point_t scheduled_time_p
        ) {
            if (match_id != match_id_ || !match_active_) {
                return;
            }

            auto* controller = controller_for_base(base_id_p);
            if (controller == nullptr || find_base(base_id_p) == nullptr) {
                return;
            }

            auto& resource = controller->state_.*resource_p;
            resource.amount_ += resource.income_;
            ++resource.income_cycle_;
            publish_controller(*controller);
            schedule_income(
                base_id_p,
                resource_p,
                scheduled_time_p + resource.income_interval_
            );
        }
    );
}

void game_t::schedule_ai_decision(
    std::uint64_t base_id_p,
    game_scheduler_t::time_point_t time_p
)
{
    scheduler_.schedule(
        time_p,
        [this, base_id_p, match_id = match_id_](
            game_scheduler_t::time_point_t scheduled_time_p
        ) {
            if (match_id != match_id_ || !match_active_) {
                return;
            }

            auto* controller = controller_for_base(base_id_p);
            if (controller == nullptr
                || controller->human_controlled_
                || find_base(base_id_p) == nullptr) {
                return;
            }

            run_ai_decision(*controller);
            schedule_ai_decision(
                base_id_p,
                scheduled_time_p + controller->profile_->decision_interval_
            );
        }
    );
}

void game_t::run_ai_decision(base_controller_t& controller_p)
{
    const auto* base = find_base(controller_p.state_.base_id_);
    if (base == nullptr || base->is_dead()) {
        return;
    }

    const auto neutral_target = find_neutral_target(controller_p);
    ai_context_t context{
        .gold_ = controller_p.state_.gold_.amount_,
        .food_ = controller_p.state_.food_.amount_,
        .base_level_ = base->level(),
        .base_health_ = base->health(),
        .base_max_health_ = base->stats().max_health_,
        .neutral_target_available_ = neutral_target.has_value()
    };
    creatures_.for_each([this, &context, base](const creature_t& creature_p) {
        if (creature_p.is_dead()) {
            return;
        }

        if (creature_p.type().group() == base->type().group()) {
            ++context.army_size_;
        } else if ((controller_for_group(creature_p.type().group()) != nullptr
                || creature_p.type().attack() > 0)
            && base->distance_to(creature_p.position()) <= base->stats().range_ + 3) {
            ++context.nearby_enemy_count_;
        }
    });

    const auto decision = ai_planner_t::plan(
        *controller_p.profile_,
        context,
        base_actions(controller_p)
    );
    controller_p.strategy_ = decision.strategy_;

    if (!decision.action_key_.empty()) {
        order_base_action(controller_p, decision.action_key_);
    }
    if (decision.launch_hunt_ && neutral_target.has_value()) {
        const auto hunting_party_size = static_cast<std::size_t>(
            std::max(1, context.army_size_ / 2)
        );
        command_attack(controller_p, *neutral_target, hunting_party_size);
    } else if (decision.launch_assault_) {
        launch_assault(controller_p);
    }

    publish_controller(controller_p);
}

std::optional<std::uint64_t> game_t::find_neutral_target(
    const base_controller_t& controller_p
) const
{
    const auto* base = find_base(controller_p.state_.base_id_);
    if (base == nullptr) {
        return std::nullopt;
    }

    std::optional<std::uint64_t> target_id;
    auto target_distance = std::numeric_limits<int>::max();
    creatures_.for_each([this, base, &controller_p, &target_id, &target_distance](
        const creature_t& creature_p
    ) {
        if (creature_p.is_dead()
            || creature_p.type().group() == controller_p.state_.base_group_) {
            return;
        }

        const auto controlled_group = std::any_of(
            controllers_.begin(),
            controllers_.end(),
            [&creature_p](const auto& candidate_p) {
                return candidate_p->state_.base_group_ == creature_p.type().group();
            }
        );
        const auto distance = base->distance_to(creature_p.position());
        if (!controlled_group
            && distance <= controller_p.profile_->hunt_maximum_distance_
            && distance < target_distance) {
            target_id = creature_p.id();
            target_distance = distance;
        }
    });
    return target_id;
}

void game_t::command_attack(
    base_controller_t& controller_p,
    std::uint64_t target_id_p,
    std::size_t maximum_unit_count_p
)
{
    if (controller_p.strategic_target_id_ != target_id_p) {
        controller_p.strategic_target_id_ = target_id_p;
        controller_p.commanded_units_.clear();
    }

    auto commanded_count = controller_p.commanded_units_.size();
    creatures_.for_each([this, &controller_p, target_id_p, maximum_unit_count_p, &commanded_count](
        creature_t& creature_p
    ) {
        if (commanded_count >= maximum_unit_count_p
            || creature_p.is_dead()
            || creature_p.type().group() != controller_p.state_.base_group_
            || controller_p.commanded_units_.contains(creature_p.id())
            || !find_commanded_target(creature_p, target_id_p).has_value()) {
            return;
        }

        creature_p.command_attack(target_id_p);
        creature_p.on_think(*this);
        controller_p.commanded_units_.insert(creature_p.id());
        ++commanded_count;
    });
    report_target_changes();
}

void game_t::launch_assault(base_controller_t& controller_p)
{
    const auto* source = find_base(controller_p.state_.base_id_);
    if (source == nullptr) {
        return;
    }

    const base_t* target = nullptr;
    auto target_distance = std::numeric_limits<int>::max();
    for (const auto& candidate : bases_) {
        if (candidate->is_dead()
            || !candidate->type().is_match_base()
            || candidate->type().group() == source->type().group()) {
            continue;
        }

        const auto distance = position_distance(source->position(), candidate->position());
        if (distance < target_distance) {
            target = candidate.get();
            target_distance = distance;
        }
    }

    if (target != nullptr) {
        command_attack(
            controller_p,
            target->id(),
            std::numeric_limits<std::size_t>::max()
        );
    }
}

void game_t::dispatch_tick()
{
#ifndef NDEBUG
    const auto tick_start = std::chrono::steady_clock::now();
#endif

    creatures_.on_think(*this);
    creatures_.on_attacking(*this, game_constants::tick_interval);

    for (const auto& base : bases_) {
        base->on_attacking(*this, game_constants::tick_interval);
    }

    regenerate_bases();
    heal_near_bases();
    remove_dead_creatures();
    remove_dead_bases();
    report_target_changes();

    observer_->on_game_tick();

#ifndef NDEBUG
    update_debug_monitor(tick_start);
#endif
}

void game_t::dispatch_movement(
    std::chrono::steady_clock::time_point now_p
)
{
    creatures_.update_movement(*this, now_p);
    report_target_changes();
}

#ifndef NDEBUG
void game_t::update_debug_monitor(
    std::chrono::steady_clock::time_point tick_start_p
) const
{
    debug_console::update_monitor({
        .tick_duration_ = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - tick_start_p
        ),
        .creature_count_ = creatures_.size(),
        .occupied_position_count_ = game_map_.occupied_position_count(),
        .position_count_ = game_map_t::position_count,
        .pending_event_count_ = dispatcher_.pending_event_count(),
        .peak_pending_event_count_ = dispatcher_.peak_pending_event_count(),
        .dispatched_event_count_ = dispatcher_.dispatched_event_count()
    });
}
#endif

void game_t::request_walk_to(
    std::uint64_t id_p,
    position_t destination_p
)
{
    post([this, id_p, destination_p]() {
        set_creature_destination(id_p, destination_p);
    });
}

void game_t::request_set_aggressive(bool aggressive_p)
{
    post([this, aggressive_p]() {
        set_aggressive(aggressive_p);
    });
}

void game_t::request_set_time_scale(double time_scale_p)
{
    post([this, time_scale_p]() {
        set_time_scale(time_scale_p);
    });
}

void game_t::set_creature_destination(
    std::uint64_t id_p,
    position_t destination_p
)
{
    auto* creature = creatures_.find(id_p);

    if (creature != nullptr && is_player_creature(*creature)) {
        creature->request_walk(destination_p);
        observer_->on_creature_walk_requested(id_p);
        creature->on_think(*this);
        report_target_changes();
    }
}

void game_t::request_attack_target(std::uint64_t id_p, std::uint64_t target_id_p)
{
    post([this, id_p, target_id_p]() {
        set_attack_target(id_p, target_id_p);
    });
}

void game_t::set_attack_target(std::uint64_t id_p, std::uint64_t target_id_p)
{
    auto* creature = creatures_.find(id_p);

    if (creature == nullptr
        || !is_player_creature(*creature)
        || !find_commanded_target(*creature, target_id_p).has_value()) {
        return;
    }

    creature->command_attack(target_id_p);
    creature->on_think(*this);
    report_target_changes();
}

bool game_t::is_player_creature(const creature_t& creature_p) const noexcept
{
    const auto* controller = player_controller();
    return controller == nullptr
        || creature_p.type().group() == controller->state_.base_group_;
}

void game_t::report_target_changes()
{
    creatures_.for_each([this](creature_t& creature_p) {
        if (creature_p.take_target_change()) {
            observer_->on_creature_target_changed(
                creature_p.id(),
                creature_p.target_id().value_or(0)
            );
        }
    });
}

std::optional<target_t> game_t::find_commanded_target(
    const creature_t& creature_p,
    std::uint64_t target_id_p
) const noexcept
{
    const auto& group = creature_p.type().group();

    if (const auto* target = creatures_.find(target_id_p); target != nullptr) {
        if (target->is_dead() || target->type().group() == group) {
            return std::nullopt;
        }

        return target_t{target->id(), target->position()};
    }

    const auto* base = find_base(target_id_p);

    if (base == nullptr || base->is_dead() || base->type().group() == group) {
        return std::nullopt;
    }

    return target_t{base->id(), base->closest_position_to(creature_p.position())};
}

void game_t::set_aggressive(bool aggressive_p)
{
    if (aggressive_ == aggressive_p) {
        return;
    }

    aggressive_ = aggressive_p;
    creatures_.on_think(*this);
}

void game_t::set_time_scale(double time_scale_p)
{
    if (game_clock_.time_scale() == time_scale_p) {
        return;
    }

    game_clock_.set_time_scale(time_scale_p);
    publish_base_actions();
}

std::optional<target_t> game_t::find_nearest_visible_enemy(
    const creature_t& creature_p
) const noexcept
{
    std::optional<target_t> nearest_enemy;
    auto nearest_distance = std::numeric_limits<int>::max();
    const auto consider = [&](std::uint64_t id_p) {
        const auto enemy = find_visible_enemy(creature_p, id_p);

        if (!enemy.has_value()) {
            return;
        }

        const auto distance = position_distance(
            creature_p.position(),
            enemy->position_
        );

        if (distance < nearest_distance) {
            nearest_enemy = enemy;
            nearest_distance = distance;
        }
    };

    for (const auto visible_id : creature_p.visible_creature_ids()) {
        consider(visible_id);
    }

    for (const auto& base : bases_) {
        consider(base->id());
    }

    return nearest_enemy;
}

std::optional<target_t> game_t::find_visible_enemy(
    const creature_t& creature_p,
    std::uint64_t target_id_p
) const noexcept
{
    const auto& group = creature_p.type().group();

    if (creature_p.visible_creature_ids().contains(target_id_p)) {
        const auto* target = creatures_.find(target_id_p);

        if (target == nullptr
            || target->is_dead()
            || target->type().group() == group) {
            return std::nullopt;
        }

        return target_t{target->id(), target->position()};
    }

    const auto* base = find_base(target_id_p);

    if (base == nullptr
        || base->is_dead()
        || base->type().group() == group
        || (creature_p.type().identifier() == "wolf"
            && base->type().identifier() == "troll_lair")
        || base->distance_to(creature_p.position())
            > creature_p.type().vision_range()) {
        return std::nullopt;
    }

    return target_t{base->id(), base->closest_position_to(creature_p.position())};
}

std::optional<target_t> game_t::find_nearest_visible_threat(
    const creature_t& creature_p
) const noexcept
{
    std::optional<target_t> nearest_threat;
    auto nearest_distance = std::numeric_limits<int>::max();

    for (const auto visible_id : creature_p.visible_creature_ids()) {
        const auto threat = find_visible_threat(creature_p, visible_id);

        if (!threat.has_value()) {
            continue;
        }

        const auto distance = position_distance(
            creature_p.position(),
            threat->position_
        );

        if (distance < nearest_distance) {
            nearest_threat = threat;
            nearest_distance = distance;
        }
    }

    return nearest_threat;
}

std::optional<target_t> game_t::find_visible_threat(
    const creature_t& creature_p,
    std::uint64_t target_id_p
) const noexcept
{
    if (!creature_p.visible_creature_ids().contains(target_id_p)) {
        return std::nullopt;
    }

    const auto* target = creatures_.find(target_id_p);

    if (target == nullptr
        || target->is_dead()
        || target->type().group() == creature_p.type().group()
        || target->type().attack() == 0) {
        return std::nullopt;
    }

    return target_t{target->id(), target->position()};
}

bool game_t::is_in_attack_range(
    const creature_t& creature_p,
    const target_t& target_p
) const noexcept
{
    return position_distance(creature_p.position(), target_p.position_)
        <= creature_p.type().attack_range();
}

std::optional<target_t> game_t::find_base_target(
    const base_t& base_p,
    std::uint64_t target_id_p
) const noexcept
{
    const auto& group = base_p.type().group();
    const auto range = base_p.stats().range_;

    if (const auto* creature = creatures_.find(target_id_p); creature != nullptr) {
        if (creature->is_dead()
            || creature->type().group() == group
            || base_p.distance_to(creature->position()) > range) {
            return std::nullopt;
        }

        return target_t{creature->id(), creature->position()};
    }

    const auto* base = find_base(target_id_p);

    if (base == nullptr
        || base->is_dead()
        || base->type().group() == group
        || base_p.distance_to(*base) > range) {
        return std::nullopt;
    }

    return target_t{
        base->id(),
        base->closest_position_to(base_p.closest_position_to(base->position()))
    };
}

std::optional<target_t> game_t::find_nearest_base_target(
    const base_t& base_p
) const noexcept
{
    std::optional<target_t> nearest_target;
    auto nearest_distance = std::numeric_limits<int>::max();
    const auto consider = [&](std::uint64_t id_p) {
        const auto target = find_base_target(base_p, id_p);

        if (!target.has_value()) {
            return;
        }

        const auto distance = base_p.distance_to(target->position_);

        if (distance < nearest_distance) {
            nearest_target = target;
            nearest_distance = distance;
        }
    };

    creatures_.for_each([&consider](const creature_t& creature_p) {
        consider(creature_p.id());
    });

    for (const auto& base : bases_) {
        consider(base->id());
    }

    return nearest_target;
}

void game_t::perform_base_attack(const base_t& base_p, const target_t& target_p)
{
    observer_->on_base_attack_performed(base_p.id(), target_p.position_);
    damage_target(
        target_p.id_,
        base_p.stats().attack_,
        base_p.type().group()
    );
}

bool game_t::move_creature_towards(
    std::uint64_t id_p,
    position_t destination_p
)
{
    auto* creature = creatures_.find(id_p);

    if (creature == nullptr) {
        return false;
    }

    const auto next_position = game_map_.next_step_towards(
        *creature,
        destination_p
    );

    if (!next_position.has_value()) {
        return false;
    }

    return move_creature(*creature, *next_position);
}

bool game_t::move_creature_away_from(
    std::uint64_t id_p,
    position_t threat_position_p
)
{
    auto* creature = creatures_.find(id_p);

    if (creature == nullptr) {
        return false;
    }

    const auto next_position = game_map_.next_step_away(
        *creature,
        threat_position_p
    );

    if (!next_position.has_value()) {
        return false;
    }

    return move_creature(*creature, *next_position);
}

bool game_t::move_creature_idle(
    std::uint64_t id_p,
    position_t idle_starting_position_p
)
{
    auto* creature = creatures_.find(id_p);

    if (creature == nullptr) {
        return false;
    }

    const auto next_position = game_map_.next_idle_step(
        *creature,
        idle_starting_position_p
    );

    if (!next_position.has_value()) {
        return false;
    }

    return move_creature(*creature, *next_position);
}

bool game_t::move_creature(
    creature_t& creature_p,
    position_t position_p
)
{
    const auto previous_position = creature_p.position();

    if (!game_map_.move_creature(creature_p, position_p)) {
        return true;
    }

    observer_->on_creature_moved(
        creature_p.id(),
        creature_p.position(),
        creature_p.direction()
    );
    visibility_system_.move_creature(
        creature_p,
        previous_position,
        game_map_,
        creatures_,
        [this](std::uint64_t observer_id_p, std::uint64_t spotted_id_p) {
            notify_creature_spotted(observer_id_p, spotted_id_p);
        }
    );

    return true;
}

void game_t::notify_creature_spotted(
    std::uint64_t observer_id_p,
    std::uint64_t spotted_id_p
)
{
    const auto* observing_creature = creatures_.find(observer_id_p);
    const auto* spotted_creature = creatures_.find(spotted_id_p);

    if (observing_creature == nullptr
        || spotted_creature == nullptr
        || observing_creature->type().group()
            == spotted_creature->type().group()) {
        return;
    }

    observer_->on_creature_spotted(observer_id_p, spotted_id_p);
}

bool game_t::check_creature_attack(
    std::uint64_t attacker_id_p,
    std::uint64_t target_id_p
)
{
    const auto* attacker = creatures_.find(attacker_id_p);

    if (attacker == nullptr || attacker->is_dead()) {
        return false;
    }

    const auto target = find_visible_enemy(*attacker, target_id_p);

    if (!target.has_value() || !is_in_attack_range(*attacker, *target)) {
        return false;
    }

    observer_->on_creature_attack_performed(attacker->id(), target->position_);

    if (attacker->type().area_radius() > 0) {
        damage_area(*attacker, target->position_, attacker->type().area_radius());
    } else {
        damage_target(
            target_id_p,
            attacker->type().attack(),
            attacker->type().group()
        );
    }

    return true;
}

void game_t::damage_target(
    std::uint64_t target_id_p,
    int damage_p,
    const std::string& attacker_group_p
)
{
    if (auto* creature = creatures_.find(target_id_p); creature != nullptr) {
        const auto previous_health = creature->health();
        creature->receive_damage(attacker_group_p, damage_p);

        if (creature->health() != previous_health) {
            creature->register_hit(current_time());
            observer_->on_creature_health_changed(creature->id(), creature->health());
        }

        if (creature->is_dead()) {
            dead_creature_ids_.push_back(creature->id());
        }

        return;
    }

    if (auto* base = find_base(target_id_p); base != nullptr) {
        const auto previous_health = base->health();
        base->receive_damage(attacker_group_p, damage_p);

        if (base->health() != previous_health) {
            observer_->on_base_health_changed(base->id(), base->health());
        }
    }
}

resource_reward_t game_t::award_reward(
    const resource_reward_t& reward_p,
    const damage_contributions_t& contributions_p
)
{
    std::unordered_map<std::string, int> eligible_damage;

    for (const auto& [group, damage] : contributions_p.by_group()) {
        if (earned_resources_.contains(group)) {
            eligible_damage.emplace(group, damage);
        }
    }

    std::unordered_set<std::uint64_t> rewarded_base_ids;
    resource_reward_t player_reward;
    const auto award = [
        this,
        &eligible_damage,
        &rewarded_base_ids,
        &player_reward
    ](
        int amount_p,
        int resource_reward_t::* resource_p
    ) {
        for (const auto& share : split_reward(amount_p, eligible_damage)) {
            earned_resources_[share.group_].*resource_p += share.amount_;

            auto* controller = controller_for_group(share.group_);
            if (controller != nullptr && share.amount_ > 0) {
                auto& resource = resource_p == &resource_reward_t::gold_
                    ? controller->state_.gold_.amount_
                    : controller->state_.food_.amount_;
                resource += share.amount_;
                rewarded_base_ids.insert(controller->state_.base_id_);

                if (controller->human_controlled_) {
                    player_reward.*resource_p += share.amount_;
                }
            }
        }
    };

    award(reward_p.gold_, &resource_reward_t::gold_);
    award(reward_p.food_, &resource_reward_t::food_);

    for (const auto base_id : rewarded_base_ids) {
        if (const auto* controller = controller_for_base(base_id); controller != nullptr) {
            publish_controller(*controller);
        }
    }
    if (!rewarded_base_ids.empty()) {
        publish_base_actions();
    }

    return player_reward;
}

void game_t::create_corpse(const creature_t& creature_p)
{
    if (!corpse_ids_.insert(creature_p.id()).second) {
        return;
    }

    observer_->on_corpse_created(
        creature_p.id(),
        creature_p.position(),
        creature_p.type().identifier()
    );
    scheduler_.schedule(
        game_clock_.now() + corpse_lifetime,
        [this, id = creature_p.id(), match_id = match_id_](auto) {
            if (match_id == match_id_) {
                remove_corpse(id);
            }
        }
    );
}

void game_t::remove_corpse(std::uint64_t id_p)
{
    if (corpse_ids_.erase(id_p) > 0) {
        observer_->on_corpse_removed(id_p);
    }
}

void game_t::remove_dead_bases()
{
    std::vector<std::string> eliminated_groups;

    std::erase_if(bases_, [this, &eliminated_groups](const auto& base_p) {
        if (!base_p->is_dead()) {
            return false;
        }

        const auto reward = award_reward(
            base_p->type().reward(),
            base_p->damage_contributions()
        );

        if (reward.gold_ > 0 || reward.food_ > 0) {
            auto reward_position = base_p->position();
            const auto center_offset = base_p->type().size() / 2;
            reward_position.column_ += center_offset;
            reward_position.row_ += center_offset;
            observer_->on_resource_rewarded(
                reward_position,
                reward.gold_,
                reward.food_
            );
        }

        if (base_p->type().is_match_base()) {
            eliminated_groups.push_back(base_p->type().group());
        }

        game_map_.remove_base(*base_p);
        observer_->on_base_removed(base_p->id());
        return true;
    });

    if (!match_active_ || eliminated_groups.empty()) {
        return;
    }

    for (const auto& group : eliminated_groups) {
        eliminate_group(group);
    }

    check_match_end();
}

void game_t::eliminate_group(const std::string& group_p)
{
    creatures_.for_each([this, &group_p](creature_t& creature_p) {
        if (creature_p.is_dead() || creature_p.type().group() != group_p) {
            return;
        }

        creature_p.receive_damage(group_p, creature_p.health());
        observer_->on_creature_health_changed(creature_p.id(), 0);
        dead_creature_ids_.push_back(creature_p.id());
    });
}

void game_t::check_match_end()
{
    const auto* controller = player_controller();
    if (controller == nullptr) {
        return;
    }

    const auto& player_group = controller->state_.base_group_;
    const auto player_base_alive = std::any_of(
        bases_.begin(),
        bases_.end(),
        [&player_group](const auto& base_p) {
            return base_p->type().is_match_base()
                && base_p->type().group() == player_group;
        }
    );
    const auto enemy_base_alive = std::any_of(
        bases_.begin(),
        bases_.end(),
        [&player_group](const auto& base_p) {
            return base_p->type().is_match_base()
                && base_p->type().group() != player_group;
        }
    );

    if (!player_base_alive) {
        end_match(false);
    } else if (!enemy_base_alive) {
        end_match(true);
    }
}

void game_t::end_match(bool victory_p)
{
    match_active_ = false;
    wildlife_spawner_.reset();
    for (auto& controller : controllers_) {
        controller->orders_->clear();
    }
    publish_base_actions();
    observer_->on_match_ended(
        victory_p,
        std::chrono::duration_cast<std::chrono::milliseconds>(
            current_time() - match_started_at_
        )
    );
}

void game_t::remove_dead_creatures()
{
    for (const auto id : dead_creature_ids_) {
        auto* creature = creatures_.find(id);

        if (creature == nullptr || !creature->is_dead()) {
            continue;
        }

        if (!game_map_.remove_creature(*creature)) {
            continue;
        }

        const auto reward = award_reward(
            creature->type().reward(),
            creature->damage_contributions()
        );

        if (reward.gold_ > 0 || reward.food_ > 0) {
            observer_->on_resource_rewarded(
                creature->position(),
                reward.gold_,
                reward.food_
            );
        }

        create_corpse(*creature);
        visibility_system_.remove_creature(*creature, creatures_);
        observer_->on_creature_removed(id);
        creatures_.remove(id);
    }

    dead_creature_ids_.clear();
}

void game_t::notify_creature_state_changed(const creature_t& creature_p)
{
    observer_->on_creature_state_changed(
        creature_p.id(),
        creature_p.state()
    );
}

void game_t::publish_creatures()
{
    creatures_.for_each([this](const creature_t& creature_p) {
        observer_->on_creature_created(creature_p);
    });

    for (const auto& base : bases_) {
        observer_->on_base_created(*base);
    }

    if (const auto* controller = player_controller(); controller != nullptr) {
        publish_controller(*controller);
    }
}
