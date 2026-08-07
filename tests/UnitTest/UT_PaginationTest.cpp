#include <iostream>
#include "../TestHelper.h"
#include "dto/Pagination.h"

// ---------------------------------------------------------
// HAPPY PATH TESTS
// ---------------------------------------------------------
void UT_Pagination_TotalPages_CalculatesCorrectly() {
    // Arrange
    PagedResult<int> result;
    result.totalCount = 100;
    result.pageSize = 20;
    result.page = 1;
    
    // Act
    int totalPages = result.totalPages();
    
    // Assert
    TEST_ASSERT_TRUE(totalPages == 5);
}

void UT_Pagination_HasNext_ReturnsTrueOnFirstPage() {
    // Arrange
    PagedResult<int> result;
    result.totalCount = 50;
    result.pageSize = 20;
    result.page = 1;
    
    // Act & Assert
    TEST_ASSERT_TRUE(result.hasNext() == true);
    TEST_ASSERT_TRUE(result.hasPrev() == false);
}

void UT_Pagination_HasPrev_ReturnsTrueOnSecondPage() {
    // Arrange
    PagedResult<int> result;
    result.totalCount = 50;
    result.pageSize = 20;
    result.page = 2;
    
    // Act & Assert
    TEST_ASSERT_TRUE(result.hasPrev() == true);
    TEST_ASSERT_TRUE(result.hasNext() == true);
}

void UT_Pagination_LastPage_HasNoNext() {
    // Arrange
    PagedResult<int> result;
    result.totalCount = 50;
    result.pageSize = 20;
    result.page = 3; // Last page (50/20 = 3 pages)
    
    // Act & Assert
    TEST_ASSERT_TRUE(result.hasNext() == false);
    TEST_ASSERT_TRUE(result.hasPrev() == true);
}

// ---------------------------------------------------------
// EDGE CASE TESTS
// ---------------------------------------------------------
void UT_Pagination_TotalPages_RoundsUpCorrectly() {
    // Arrange — 21 records / 20 per page = 2 pages (ceil)
    PagedResult<int> result;
    result.totalCount = 21;
    result.pageSize = 20;
    result.page = 1;
    
    // Act & Assert
    TEST_ASSERT_TRUE(result.totalPages() == 2);
}

void UT_Pagination_ExactlyOnePage_NoNavigation() {
    // Arrange — exactly one page
    PagedResult<int> result;
    result.totalCount = 20;
    result.pageSize = 20;
    result.page = 1;
    
    // Act & Assert
    TEST_ASSERT_TRUE(result.totalPages() == 1);
    TEST_ASSERT_TRUE(result.hasNext() == false);
    TEST_ASSERT_TRUE(result.hasPrev() == false);
}

void UT_Pagination_EmptyResult_IsEmpty() {
    // Arrange
    PagedResult<int> result;
    result.totalCount = 0;
    result.pageSize = 20;
    result.page = 1;
    
    // Act & Assert
    TEST_ASSERT_TRUE(result.totalPages() == 0);
    TEST_ASSERT_TRUE(result.isEmpty() == true);
    TEST_ASSERT_TRUE(result.hasNext() == false);
    TEST_ASSERT_TRUE(result.hasPrev() == false);
}

// ---------------------------------------------------------
// NEGATIVE CASE TESTS
// ---------------------------------------------------------
void UT_Pagination_ZeroPageSize_ReturnsZeroPages() {
    // Arrange — pageSize = 0 is a guard case
    PagedResult<int> result;
    result.totalCount = 100;
    result.pageSize = 0;
    result.page = 1;
    
    // Act & Assert
    TEST_ASSERT_TRUE(result.totalPages() == 0);
}

void UT_Pagination_NegativePageSize_ReturnsZeroPages() {
    // Arrange
    PagedResult<int> result;
    result.totalCount = 100;
    result.pageSize = -5;
    result.page = 1;
    
    // Act & Assert
    TEST_ASSERT_TRUE(result.totalPages() == 0);
}

void runPaginationTestSuite() {
    std::cout << "  [Suite 10/11] Running Pagination Logic Tests..." << std::endl;
    RUN_TEST_CASE(UT_Pagination_TotalPages_CalculatesCorrectly);
    RUN_TEST_CASE(UT_Pagination_HasNext_ReturnsTrueOnFirstPage);
    RUN_TEST_CASE(UT_Pagination_HasPrev_ReturnsTrueOnSecondPage);
    RUN_TEST_CASE(UT_Pagination_LastPage_HasNoNext);
    RUN_TEST_CASE(UT_Pagination_TotalPages_RoundsUpCorrectly);
    RUN_TEST_CASE(UT_Pagination_ExactlyOnePage_NoNavigation);
    RUN_TEST_CASE(UT_Pagination_EmptyResult_IsEmpty);
    RUN_TEST_CASE(UT_Pagination_ZeroPageSize_ReturnsZeroPages);
    RUN_TEST_CASE(UT_Pagination_NegativePageSize_ReturnsZeroPages);
    std::cout << "    ==> [PASSED] All Pagination Test Cases (9/9)" << std::endl;
    std::fflush(stdout);
}
