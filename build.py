#!/usr/bin/python3

import subprocess
from pathlib import Path

CURRENT_DIR = Path(__file__).parent

def main() -> int:
    scetool_dir = CURRENT_DIR / 'source/oscetool'
    the_files = [Path(scetool_dir,x).resolve() for x in (scetool_dir / 'me_files.txt').read_text().split('\n')]
    
    for x in the_files:
        include_dir = x.parent.parent
        x.rename(include_dir / x.name) # moves the files up one dir to source folder
    
    try:
        subprocess.run(('make','pkg'))
    finally:
        for x in the_files:
            new_x = x.parent.parent  / x.name
            new_x.rename(x) # now bring it back
    
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
