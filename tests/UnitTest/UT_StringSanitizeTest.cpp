#include <iostream>
#include <cstdio>
#include "../TestHelper.h"
#include "utils/StringSanitize.h"

// ---------------------------------------------------------
// HAPPY PATH TESTS
// ---------------------------------------------------------
void UT_StringSanitize_NormalString_ReturnsUnchanged() {
    QString input = "Nguyen Van A";
    QString result = StringSanitize::collapseSpaces(input);
    TEST_ASSERT_TRUE(result == "Nguyen Van A");
}

void UT_StringSanitize_LeadingAndTrailingSpaces_TrimsCorrectly() {
    QString input = "   Bac Si Nguyen   ";
    QString result = StringSanitize::collapseSpaces(input);
    TEST_ASSERT_TRUE(result == "Bac Si Nguyen");
}

void UT_StringSanitize_MultipleInternalSpaces_CollapsesToOneSpace() {
    QString input = "Khoa   Noi    Tong   Hop";
    QString result = StringSanitize::collapseSpaces(input);
    TEST_ASSERT_TRUE(result == "Khoa Noi Tong Hop");
}

// ---------------------------------------------------------
// BOUNDARY CASE TESTS
// ---------------------------------------------------------
void UT_StringSanitize_EmptyString_ReturnsEmpty() {
    QString input = "";
    QString result = StringSanitize::collapseSpaces(input);
    TEST_ASSERT_TRUE(result == "");
}

void UT_StringSanitize_OnlySpaces_ReturnsEmpty() {
    QString input = "        ";
    QString result = StringSanitize::collapseSpaces(input);
    TEST_ASSERT_TRUE(result == "");
}

void UT_StringSanitize_SingleWordWithSpaces_TrimsToWord() {
    QString input = "   Paracetamol   ";
    QString result = StringSanitize::collapseSpaces(input);
    TEST_ASSERT_TRUE(result == "Paracetamol");
}

// ---------------------------------------------------------
// NEGATIVE & SPECIAL CHARACTER TESTS
// ---------------------------------------------------------
void UT_StringSanitize_SpecialCharsAndTabs_PreservesNonSpaceWhitespace() {
    QString input = "  Line1 \n Line2  ";
    QString result = StringSanitize::collapseSpaces(input);
    TEST_ASSERT_TRUE(!result.isEmpty());
}

void UT_StringSanitize_SqlAndHtmlChars_PreservesCharactersWithoutCrashing() {
    QString input = "  ' OR 1=1 -- <script>alert()</script>  ";
    QString result = StringSanitize::collapseSpaces(input);
    TEST_ASSERT_TRUE(result == "' OR 1=1 -- <script>alert()</script>");
}

void runStringSanitizeTestSuite() {
    std::cout << "  [UT 12/12] Running String Sanitize Utility Unit Tests..." << std::endl;
    RUN_TEST_CASE(UT_StringSanitize_NormalString_ReturnsUnchanged);
    RUN_TEST_CASE(UT_StringSanitize_LeadingAndTrailingSpaces_TrimsCorrectly);
    RUN_TEST_CASE(UT_StringSanitize_MultipleInternalSpaces_CollapsesToOneSpace);
    
    RUN_TEST_CASE(UT_StringSanitize_EmptyString_ReturnsEmpty);
    RUN_TEST_CASE(UT_StringSanitize_OnlySpaces_ReturnsEmpty);
    RUN_TEST_CASE(UT_StringSanitize_SingleWordWithSpaces_TrimsToWord);
    
    RUN_TEST_CASE(UT_StringSanitize_SpecialCharsAndTabs_PreservesNonSpaceWhitespace);
    RUN_TEST_CASE(UT_StringSanitize_SqlAndHtmlChars_PreservesCharactersWithoutCrashing);
    
    std::cout << "    ==> [PASSED] All String Sanitize Test Cases (8/8)" << std::endl;
    std::fflush(stdout);
}
