import argparse
import math
import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
EXTRACTED = ROOT / "assets" / "models" / "extracted"
RUNTIME = ROOT / "assets" / "models" / "runtime"


MODELS = [
    {
        "name": "office_chair_modern",
        "source": EXTRACTED / "Office_Chair_v1_L3.123cf4a20c81-89b5-417b-848a-24ea67f9fb6b" / "10239_Office_Chair_v1_L3.obj",
        "target": 12000,
    },
    {
        "name": "office_chair_classic",
        "source": EXTRACTED / "Office chair.obj",
        "target": 12000,
    },
    {
        "name": "office_chair_task",
        "source": EXTRACTED / "Office Chair.fbx",
        "target": 12000,
    },
    {
        "name": "filing_cabinet",
        "source": EXTRACTED / "Filing cabinet OBJ.obj",
        "target": 900,
    },
    {
        "name": "office_desk",
        "source": EXTRACTED / "desk.obj",
        "target": 1000,
    },
    {
        "name": "trashbin",
        "source": ROOT / "assets" / "models" / "trashbin.fbx",
        "target": 1600,
    },
    {
        "name": "desklamp",
        "source": ROOT / "assets" / "models" / "desklamp" / "77763.obj",
        "target": 1800,
    },
    {
        "name": "audio_caset",
        "source": ROOT / "assets" / "models" / "audio_caset" / "cassette_audio_4_track_ericdesign.obj",
        "target": 1600,
    },
]


def ensure_extracted() -> None:
    archives = [
        ROOT / "assets" / "models" / "Office_Chair_v1_L3.123cf4a20c81-89b5-417b-848a-24ea67f9fb6b.zip",
        ROOT / "assets" / "models" / "65-archiv.zip",
        ROOT / "assets" / "models" / "67-filing-cabinet-obj.zip",
        ROOT / "assets" / "models" / "20-office-desk.rar",
        ROOT / "assets" / "models" / "19-office-chair.rar",
    ]
    EXTRACTED.mkdir(parents=True, exist_ok=True)
    for archive in archives:
        if not archive.exists():
            continue
        subprocess.run(["tar", "-xf", str(archive), "-C", str(EXTRACTED)], check=True)


def find_blender() -> Path:
    from_path = shutil.which("blender") or shutil.which("blender.exe")
    if from_path:
        return Path(from_path)

    candidates = [
        Path("C:/Program Files/Blender Foundation"),
        Path("C:/Program Files (x86)/Blender Foundation"),
        Path.home() / "AppData" / "Local" / "Programs",
    ]
    found = []
    for base in candidates:
        if base.exists():
            found.extend(base.rglob("blender.exe"))
    if not found:
        raise FileNotFoundError("blender.exe was not found")
    return sorted(found, key=lambda p: p.stat().st_mtime, reverse=True)[0]


def run_blender(blender: Path) -> None:
    script = Path(__file__).with_name("prepare-prop-models.blender.py")
    subprocess.run(
        [
            str(blender),
            "--background",
            "--factory-startup",
            "--python",
            str(script),
            "--",
            str(EXTRACTED),
            str(RUNTIME),
        ],
        check=True,
    )


def convert_runtime_models() -> None:
    script = Path(__file__).with_name("convert-runtime-models.py")
    subprocess.run([sys.executable, str(script)], check=True)


def count_obj_triangles(path: Path) -> int:
    tris = 0
    with path.open("r", encoding="utf-8", errors="ignore") as handle:
        for line in handle:
            if line.startswith("f "):
                count = len(line.split()) - 1
                if count >= 3:
                    tris += count - 2
    return tris


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--skip-extract", action="store_true")
    args = parser.parse_args()

    if not args.skip_extract:
        ensure_extracted()

    blender = find_blender()
    print(f"Using Blender: {blender}")
    run_blender(blender)
    convert_runtime_models()

    for model in MODELS:
        output = RUNTIME / f"{model['name']}.obj"
        if output.exists():
            print(f"{output.relative_to(ROOT)}: {count_obj_triangles(output)} tris")


if __name__ == "__main__":
    main()
