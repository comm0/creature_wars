import argparse
import json
import shutil
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parent.parent
SOURCE_DIRECTIONS = ("north", "east", "south", "west")
DIRECTIONS = ("north", "south", "west", "east")
SCALES = {"": 1, "+hd": 2, "+hd2": 4}
FRAME_SIZE = 32
EXTRUDE = 1
PADDING = 2
CELL_SIZE = FRAME_SIZE + 2 * EXTRUDE + PADDING


def paste_extruded_frame(atlas, frame, x, y):
    atlas.paste(frame.crop((0, 0, FRAME_SIZE, 1)), (x, y - EXTRUDE))
    atlas.paste(
        frame.crop((0, FRAME_SIZE - 1, FRAME_SIZE, FRAME_SIZE)),
        (x, y + FRAME_SIZE),
    )
    atlas.paste(frame, (x, y))
    atlas.paste(
        atlas.crop((x, y - EXTRUDE, x + 1, y + FRAME_SIZE + EXTRUDE)),
        (x - EXTRUDE, y - EXTRUDE),
    )
    atlas.paste(
        atlas.crop(
            (
                x + FRAME_SIZE - 1,
                y - EXTRUDE,
                x + FRAME_SIZE,
                y + FRAME_SIZE + EXTRUDE,
            )
        ),
        (x + FRAME_SIZE, y - EXTRUDE),
    )


def build_atlas(source):
    image = Image.open(source)

    if image.mode != "RGBA" or image.size != (96, 128):
        raise ValueError(f"Invalid animation sheet: {source}")

    atlas = Image.new(
        "RGBA",
        (3 * CELL_SIZE + PADDING, 4 * CELL_SIZE + PADDING),
        (0, 0, 0, 0),
    )
    frames = {}

    for row, direction in enumerate(DIRECTIONS):
        source_row = SOURCE_DIRECTIONS.index(direction)

        for column in range(3):
            frame = image.crop(
                (
                    column * FRAME_SIZE,
                    source_row * FRAME_SIZE,
                    (column + 1) * FRAME_SIZE,
                    (source_row + 1) * FRAME_SIZE,
                )
            )
            x = PADDING + column * CELL_SIZE + EXTRUDE
            y = PADDING + row * CELL_SIZE + EXTRUDE
            paste_extruded_frame(atlas, frame, x, y)
            frames[f"{direction}_{column}.png"] = {
                "x": x,
                "y": y,
                "w": FRAME_SIZE,
                "h": FRAME_SIZE,
            }

    return atlas, frames


def write_atlas(identifier, atlas, frames, output_root):
    for folder, scale in SCALES.items():
        target = output_root / identifier / folder
        target.mkdir(parents=True, exist_ok=True)
        scaled_atlas = atlas.resize(
            (atlas.width * scale, atlas.height * scale),
            Image.Resampling.NEAREST,
        )
        scaled_atlas.save(target / f"{identifier}.png")
        scaled_frames = {
            name: {
                "frame": {
                    key: value * scale
                    for key, value in frame.items()
                },
                "rotated": False,
                "trimmed": False,
                "spriteSourceSize": {
                    "x": 0,
                    "y": 0,
                    "w": frame["w"] * scale,
                    "h": frame["h"] * scale,
                },
                "sourceSize": {
                    "w": frame["w"] * scale,
                    "h": frame["h"] * scale,
                },
            }
            for name, frame in frames.items()
        }
        metadata = {
            "frames": scaled_frames,
            "meta": {
                "app": "https://www.codeandweb.com/texturepacker",
                "version": "1.0",
                "image": f"{identifier}.png",
                "format": "RGBA8888",
                "size": {
                    "w": scaled_atlas.width,
                    "h": scaled_atlas.height,
                },
                "scale": str(scale),
                "smartupdate": "",
            },
        }

        with open(target / f"{identifier}.json", "w", newline="\n") as file:
            json.dump(metadata, file, indent=2)
            file.write("\n")


def write_corpse(source, identifier, output_root):
    image = Image.open(source)

    if image.mode != "RGBA" or image.size != (32, 32):
        raise ValueError(f"Invalid corpse image: {source}")

    for folder, scale in SCALES.items():
        target = output_root / identifier / folder
        target.mkdir(parents=True, exist_ok=True)

        if scale == 1:
            shutil.copyfile(source, target / "corpse.png")
            continue

        image.resize(
            (image.width * scale, image.height * scale),
            Image.Resampling.NEAREST,
        ).save(target / "corpse.png")


def import_creature(source, output_root):
    identifier = source.name
    animation = source / f"{identifier}_animation_96x128.png"
    corpse = source / f"{identifier}_corpse_32x32.png"
    atlas, frames = build_atlas(animation)
    write_atlas(identifier, atlas, frames, output_root)
    write_corpse(corpse, identifier, output_root)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument(
        "--output",
        type=Path,
        default=ROOT / "assets" / "creatures",
    )
    arguments = parser.parse_args()

    for source in sorted(arguments.source.iterdir()):
        if source.is_dir():
            import_creature(source, arguments.output)


if __name__ == "__main__":
    main()
