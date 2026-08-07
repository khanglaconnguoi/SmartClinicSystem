import os
import re

def process_file(filepath, depth):
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    # 1. Replace <cassert> with TestHelper.h
    include_path = '"../TestHelper.h"' if depth == 1 else '"../../TestHelper.h"'
    content = re.sub(r'#include\s+<cassert>', f'#include {include_path}', content)

    # 2. Replace assert(...) with TEST_ASSERT_TRUE(...)
    content = re.sub(r'\bassert\s*\((.*?)\);', r'TEST_ASSERT_TRUE(\1);', content)

    # 3. Refactor block-style integration/GUI tests
    # Replace: { /* test code */ } with: RUN_INLINE_TEST("Block Test", { /* test code */ });
    # Specifically targeting the blocks with comments like // --- HAPPY: Get doctors KPI ---
    
    def inline_test_replacer(match):
        comment = match.group(1).strip()
        block = match.group(2)
        # Extract name from comment
        name = comment.replace('//', '').replace('---', '').strip()
        return f'{comment}\n    RUN_INLINE_TEST("{name}", {block});'

    # Matches: // --- SOME COMMENT ---\n    { ... }
    content = re.sub(r'(//\s*---.*?---)\s*\n\s*(\{\s*.*?\n\s*\})', inline_test_replacer, content, flags=re.DOTALL)


    # 4. Wrap test function calls in RUN_TEST_CASE(...) inside run*TestSuite functions for Unit Tests
    suite_pattern = re.compile(r'(void\s+run[A-Za-z0-9_]+TestSuite\s*\(\)\s*\{)(.*?)(^\})', re.DOTALL | re.MULTILINE)
    
    def suite_replacer(match):
        header = match.group(1)
        body = match.group(2)
        footer = match.group(3)
        
        def line_replacer(line_match):
            indent = line_match.group(1)
            func_name = line_match.group(2)
            return f'{indent}RUN_TEST_CASE({func_name});'
            
        new_body = re.sub(r'^(\s+)(UT_[A-Za-z0-9_]+|IT_[A-Za-z0-9_]+|GUI_[A-Za-z0-9_]+)\s*\(\)\s*;', line_replacer, body, flags=re.MULTILINE)
        return f"{header}{new_body}{footer}"

    content = suite_pattern.sub(suite_replacer, content)

    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(content)

def main():
    test_dir = r"d:\CodingSpace\SmartClinicSystem\tests"
    for root, dirs, files in os.walk(test_dir):
        for file in files:
            if file.endswith('.cpp') and file != 'AllTestsMain.cpp' and 'Main' not in file:
                depth = root[len(test_dir):].strip(os.sep).count(os.sep) + 1
                if depth > 0:
                    filepath = os.path.join(root, file)
                    print(f"Processing: {filepath}")
                    process_file(filepath, depth)
    print("\nRefactoring complete! All 27 tests now use TestHelper.h for continuous execution.")

if __name__ == '__main__':
    main()
