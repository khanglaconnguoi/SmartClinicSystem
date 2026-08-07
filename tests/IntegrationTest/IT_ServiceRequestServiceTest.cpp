#include <iostream>
#include "../TestHelper.h"
#include <memory>
#include "service/ServiceRequestService.h"
#include "repository/ServiceRequestRepository.h"

void runServiceRequestServiceTestSuite() {
    std::cout << "  [IT 8/9] Running ServiceRequest Integration Tests..." << std::endl;
    auto repo = std::make_shared<ServiceRequestRepository>();
    ServiceRequestService service(repo);

    int createdRequestId = -1;

    // --- HAPPY: Create a service request ---
    RUN_INLINE_TEST("HAPPY: Create a service request", {
        ServiceRequestInputDTO input;
        input.recordId = 1;
        input.roomId = 1;
        input.doctorId = 1;
        input.serviceName = "Blood Test";

        createdRequestId = service.createRequest(input);
        TEST_ASSERT_TRUE(createdRequestId > 0);
        std::cout << "    [OK] createRequest -> requestId=" << createdRequestId << std::endl;
    });

    // --- HAPPY: Get request by ID ---
    RUN_INLINE_TEST("HAPPY: Get request by ID", {
        auto request = service.getById(createdRequestId);
        TEST_ASSERT_TRUE(request.has_value());
        TEST_ASSERT_TRUE(request->serviceName == "Blood Test");
        std::cout << "    [OK] getById -> found, serviceName=" << request->serviceName.toStdString() << std::endl;
    });

    // --- HAPPY: Cancel request ---
    RUN_INLINE_TEST("HAPPY: Cancel request", {
        bool cancelled = service.cancelRequest(createdRequestId);
        TEST_ASSERT_TRUE(cancelled == true);
        std::cout << "    [OK] cancelRequest -> success" << std::endl;
    });

    // --- NEGATIVE: Get non-existent request ---
    RUN_INLINE_TEST("NEGATIVE: Get non-existent request", {
        auto request = service.getById(999999);
        TEST_ASSERT_TRUE(!request.has_value());
        std::cout << "    [OK] getById (non-existent) -> nullopt" << std::endl;
    });

    std::cout << "    ==> [PASSED] ServiceRequest Integration Tests (4/4)" << std::endl;
    std::fflush(stdout);
}
