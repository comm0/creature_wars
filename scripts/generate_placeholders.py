"""Generates grayscale oblique-projection placeholder art (tinted in QML).

Requires Pillow. Run from the repository root:
    python scripts/generate_placeholders.py
"""

import json
from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parent.parent
SCALES = {"": 1, "+hd": 2, "+hd2": 4}

OUTLINE = (40, 40, 40, 255)
TOP = (255, 255, 255, 255)
EAST = (185, 185, 185, 255)
SOUTH = (135, 135, 135, 255)
DETAIL = (25, 25, 25, 255)


def draw_box(draw, left, top, right, bottom, height):
    lifted = [left - height, top - height, right - height, bottom - height]
    south = [
        (lifted[0], lifted[3]),
        (lifted[2], lifted[3]),
        (right, bottom),
        (left, bottom),
    ]
    east = [
        (lifted[2], lifted[1]),
        (right, top),
        (right, bottom),
        (lifted[2], lifted[3]),
    ]
    draw.polygon(south, fill=SOUTH, outline=OUTLINE)
    draw.polygon(east, fill=EAST, outline=OUTLINE)
    draw.rectangle(lifted, fill=TOP, outline=OUTLINE)
    return lifted


def draw_boxes(draw, boxes):
    for box in sorted(boxes, key=lambda box: box[2] + box[3]):
        draw_box(draw, *box)


def pawn_frame(step):
    frame = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    lift = 1 if step else 0
    draw_box(ImageDraw.Draw(frame), 19, 19 - lift, 28, 28 - lift, 10)
    return frame


def write_scaled(image, directory, name, frames=None):
    for folder, scale in SCALES.items():
        target = ROOT / directory / folder
        target.mkdir(parents=True, exist_ok=True)
        image.resize(
            (image.width * scale, image.height * scale),
            Image.NEAREST,
        ).save(target / f"{name}.png")

        if frames is None:
            continue

        scaled_frames = {
            key: {
                "frame": {k: v * scale for k, v in value.items()},
                "rotated": False,
                "trimmed": False,
                "spriteSourceSize": {
                    "x": 0,
                    "y": 0,
                    "w": value["w"] * scale,
                    "h": value["h"] * scale,
                },
                "sourceSize": {"w": value["w"] * scale, "h": value["h"] * scale},
            }
            for key, value in frames.items()
        }
        sheet = {
            "frames": scaled_frames,
            "meta": {
                "app": "https://www.codeandweb.com/texturepacker",
                "version": "1.0",
                "image": f"{name}.png",
                "format": "RGBA8888",
                "size": {"w": image.width * scale, "h": image.height * scale},
                "scale": str(scale),
                "smartupdate": "",
            },
        }
        with open(target / f"{name}.json", "w", newline="\n") as file:
            json.dump(sheet, file, indent=2)
            file.write("\n")


def generate_pawn():
    size, extrude, padding = 32, 1, 2
    cell = size + 2 * extrude + padding
    directions = ["north", "south", "west", "east"]
    sheet = Image.new(
        "RGBA",
        (3 * cell + padding, len(directions) * cell + padding),
        (0, 0, 0, 0),
    )
    frames = {}

    for row, direction in enumerate(directions):
        for column in range(3):
            frame = pawn_frame(column)
            x = padding + column * cell + extrude
            y = padding + row * cell + extrude
            sheet.paste(frame.crop((0, 0, size, 1)), (x, y - 1))
            sheet.paste(frame.crop((0, size - 1, size, size)), (x, y + size))
            sheet.paste(frame, (x, y))
            sheet.paste(sheet.crop((x, y - 1, x + 1, y + size + 1)), (x - 1, y - 1))
            sheet.paste(
                sheet.crop((x + size - 1, y - 1, x + size, y + size + 1)),
                (x + size, y - 1),
            )
            frames[f"{direction}_{column}.png"] = {"x": x, "y": y, "w": size, "h": size}

    write_scaled(sheet, "assets/creatures/placeholder", "placeholder", frames)


def draw_door(draw, top):
    draw.polygon(
        [
            (top[0] + 20, top[3] + 5),
            (top[0] + 26, top[3] + 5),
            (top[0] + 32, top[3] + 11),
            (top[0] + 26, top[3] + 11),
        ],
        fill=DETAIL,
    )


def battlements(top, first, last, step, size=3, height=3):
    boxes = []

    for offset in range(first, last - size + 1, step):
        boxes.append((top[0] + offset, top[1], top[0] + offset + size, top[1] + size, height))
        boxes.append((top[0], top[1] + offset, top[0] + size, top[1] + offset + size, height))
        boxes.append((top[0] + offset, top[3] - size, top[0] + offset + size, top[3], height))
        boxes.append((top[2] - size, top[1] + offset, top[2], top[1] + offset + size, height))

    return boxes


def base_image(wall_height, extra_height):
    footprint = 48
    offset = wall_height + extra_height
    image = Image.new("RGBA", (footprint + offset, footprint + offset), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    top = draw_box(
        draw,
        offset,
        offset,
        offset + footprint - 1,
        offset + footprint - 1,
        wall_height,
    )
    return image, draw, top


def generate_bases():
    image, _, _ = base_image(12, 0)
    write_scaled(image, "assets/structures/base", "base_1")

    image, draw, top = base_image(12, 10)
    draw_door(draw, top)
    boxes = battlements(top, 1, 48, 9)
    boxes.append((top[0] + 14, top[1] + 14, top[0] + 33, top[1] + 33, 10))
    draw_boxes(draw, boxes)
    write_scaled(image, "assets/structures/base", "base_2")

    image, draw, top = base_image(16, 12)
    draw_door(draw, top)
    boxes = battlements(top, 13, 35, 6)
    tower = 9

    for column in (0, 47 - tower):
        for row in (0, 47 - tower):
            boxes.append((
                top[0] + column,
                top[1] + row,
                top[0] + column + tower,
                top[1] + row + tower,
                7,
            ))

    boxes.append((top[0] + 17, top[1] + 17, top[0] + 30, top[1] + 30, 12))
    draw_boxes(draw, boxes)
    write_scaled(image, "assets/structures/base", "base_3")


if __name__ == "__main__":
    generate_pawn()
    generate_bases()
