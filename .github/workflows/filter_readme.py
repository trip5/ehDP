#!/usr/bin/env python3
"""
Filter README.md for PlatformIO publishing.
Removes sections not relevant to Arduino/PlatformIO users.
"""
import sys
import re

def remove_section(content, section_header):
    """Remove a section from content, from header to next '---' line."""
    pattern = re.compile(
        rf'^{re.escape(section_header)}.*?^---$',
        re.MULTILINE | re.DOTALL
    )
    return pattern.sub('', content)

def main():
    readme_path = sys.argv[1] if len(sys.argv) > 1 else 'README.md'
    
    with open(readme_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Remove sections not needed in PlatformIO package
    content = remove_section(content, '## Other Supported Platforms')
    content = remove_section(content, '## Tools')
    
    # Clean up any double blank lines that might result
    content = re.sub(r'\n{3,}', '\n\n', content)
    
    with open(readme_path, 'w', encoding='utf-8') as f:
        f.write(content)
    
    print(f"Filtered {readme_path} for PlatformIO publishing")

if __name__ == '__main__':
    main()
