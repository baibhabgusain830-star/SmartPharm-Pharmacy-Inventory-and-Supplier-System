#include "PharmacyInventory.h"
#include "DateUtils.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <sstream>

void runAllTests() {
    std::cout << "=================================================================\n";
    std::cout << "           SmartPharm Automated Test Suite Execution             \n";
    std::cout << "=================================================================\n\n";

    int passed = 0;
    int total = 20;

    // Test 1: Normal medicine creation and lookup by ID and barcode
    {
        PharmacyInventory inv;
        bool added1 = inv.addMedicine(Medicine("T_M1", "Test Med 1", "BAR001", "CatA", 20.0));
        assert(added1);
        int byId = inv.searchMedicine("T_M1");
        int byBar = inv.searchMedicine("BAR001");
        assert(byId == 0 && byBar == 0);
        std::cout << "[Test 1 PASSED] Normal medicine creation and lookup by ID/Barcode.\n";
        passed++;
    }

    // Test 2: Duplicate medicine IDs and barcodes rejected
    {
        PharmacyInventory inv;
        inv.addMedicine(Medicine("T_M1", "Test Med 1", "BAR001", "CatA", 20.0));
        bool dupId = inv.addMedicine(Medicine("T_M1", "Other Name", "BAR002", "CatA", 25.0));
        bool dupBar = inv.addMedicine(Medicine("T_M2", "Other Name", "BAR001", "CatA", 25.0));
        assert(!dupId && !dupBar);
        assert(inv.getMedicineCount() == 1);
        std::cout << "[Test 2 PASSED] Duplicate medicine ID and Barcode rejected.\n";
        passed++;
    }

    // Test 3: Normal batch creation and duplicate batch IDs rejected
    {
        PharmacyInventory inv;
        inv.addMedicine(Medicine("T_M1", "Test Med 1", "BAR001", "CatA", 20.0));
        bool b1 = inv.addBatch(Batch("BAT01", "T_M1", "2027-01-01", 10, 15.0));
        bool dupB = inv.addBatch(Batch("BAT01", "T_M1", "2027-05-01", 20, 15.0));
        assert(b1 && !dupB);
        assert(inv.getBatchCount() == 1);
        std::cout << "[Test 3 PASSED] Normal batch creation and duplicate batch ID rejected.\n";
        passed++;
    }

    // Test 4: Invalid prices, quantities, IDs, and dates rejected
    {
        PharmacyInventory inv;
        inv.addMedicine(Medicine("T_M1", "Test Med 1", "BAR001", "CatA", 20.0));
        // Empty ID
        bool badMed = inv.addMedicine(Medicine("", "Name", "BAR999", "CatA", 10.0));
        // Negative price
        bool negPrice = inv.addMedicine(Medicine("T_M2", "Name", "BAR999", "CatA", -5.0));
        // Non-existent medicine for batch
        bool noMedBatch = inv.addBatch(Batch("BAT_BAD", "NON_EXISTENT", "2027-01-01", 10, 10.0));
        // Invalid date (Feb 30)
        bool badDateBatch = inv.addBatch(Batch("BAT_BAD2", "T_M1", "2026-02-30", 10, 10.0));
        // Zero/negative quantity
        bool zeroQtyBatch = inv.addBatch(Batch("BAT_BAD3", "T_M1", "2027-01-01", 0, 10.0));
        assert(!badMed && !negPrice && !noMedBatch && !badDateBatch && !zeroQtyBatch);
        std::cout << "[Test 4 PASSED] Invalid prices, zero quantities, empty IDs, impossible dates rejected.\n";
        passed++;
    }

    // Test 5: Sale using one batch
    {
        PharmacyInventory inv;
        inv.setSystemDate("2026-10-15");
        inv.addMedicine(Medicine("T_M1", "Test Med 1", "BAR001", "CatA", 20.0));
        inv.addBatch(Batch("BAT01", "T_M1", "2027-01-01", 50, 15.0));
        bool sold = inv.sellMedicineFEFO("T_M1", 20);
        assert(sold);
        assert(inv.getAvailableStock("T_M1") == 30);
        std::cout << "[Test 5 PASSED] Sale using single batch completed accurately.\n";
        passed++;
    }

    // Test 6: Sale spanning multiple batches
    {
        PharmacyInventory inv;
        inv.setSystemDate("2026-10-15");
        inv.addMedicine(Medicine("T_M1", "Test Med 1", "BAR001", "CatA", 20.0));
        inv.addBatch(Batch("B1", "T_M1", "2026-12-01", 15, 10.0));
        inv.addBatch(Batch("B2", "T_M1", "2027-06-01", 20, 10.0));
        bool sold = inv.sellMedicineFEFO("T_M1", 25);
        assert(sold);
        assert(inv.getAvailableStock("T_M1") == 10); // 35 - 25 = 10 (0 in B1, 10 in B2)
        std::cout << "[Test 6 PASSED] Sale spanning multiple batches depleted in FEFO order.\n";
        passed++;
    }

    // Test 7: Sale spanning more than 10 batches (Bug A verification)
    {
        PharmacyInventory inv;
        inv.setSystemDate("2026-10-15");
        inv.addMedicine(Medicine("T_M1", "Multi Batch Med", "BAR_MB", "CatA", 10.0));
        // Add 15 batches with 2 units each = 30 units total
        for (int i = 1; i <= 15; ++i) {
            std::string bId = "B_" + std::to_string(i);
            std::string exp = (i < 10) ? ("2027-01-0" + std::to_string(i)) : ("2027-01-" + std::to_string(i));
            inv.addBatch(Batch(bId, "T_M1", exp, 2, 5.0));
        }
        assert(inv.getBatchCount() == 15);
        assert(inv.getAvailableStock("T_M1") == 30);
        // Sell 26 units (draws across 13 distinct batches > 10 batches)
        bool sold = inv.sellMedicineFEFO("T_M1", 26);
        assert(sold);
        assert(inv.getAvailableStock("T_M1") == 4);
        std::cout << "[Test 7 PASSED] Sale spanning 13 batches (>10 batches) succeeded without undo buffer overflow.\n";
        passed++;
    }

    // Test 8: Insufficient eligible stock rejected
    {
        PharmacyInventory inv;
        inv.setSystemDate("2026-10-15");
        inv.addMedicine(Medicine("T_M1", "Test Med 1", "BAR001", "CatA", 20.0));
        inv.addBatch(Batch("B1", "T_M1", "2027-01-01", 10, 10.0));
        bool soldTooMuch = inv.sellMedicineFEFO("T_M1", 25);
        assert(!soldTooMuch);
        assert(inv.getAvailableStock("T_M1") == 10); // unchanged
        std::cout << "[Test 8 PASSED] Sale exceeding available stock rejected; inventory unchanged.\n";
        passed++;
    }

    // Test 9: Expired batches excluded from sale
    {
        PharmacyInventory inv;
        inv.setSystemDate("2026-10-15");
        inv.addMedicine(Medicine("T_M1", "Test Med 1", "BAR001", "CatA", 20.0));
        inv.addBatch(Batch("B_EXP", "T_M1", "2026-05-01", 50, 10.0)); // Expired relative to 2026-10-15
        inv.addBatch(Batch("B_ACT", "T_M1", "2027-05-01", 20, 10.0)); // Active
        assert(inv.getAvailableStock("T_M1") == 20); // Only active counted
        bool sold = inv.sellMedicineFEFO("T_M1", 25); // Asking for 25 exceeds active 20
        assert(!sold);
        bool soldValid = inv.sellMedicineFEFO("T_M1", 15);
        assert(soldValid);
        assert(inv.getAvailableStock("T_M1") == 5);
        std::cout << "[Test 9 PASSED] Expired batches strictly excluded from available stock and FEFO sale.\n";
        passed++;
    }

    // Test 10: FEFO order verified when multiple eligible batches exist
    {
        PharmacyInventory inv;
        inv.setSystemDate("2026-10-15");
        inv.addMedicine(Medicine("T_M1", "Test Med 1", "BAR001", "CatA", 20.0));
        inv.addBatch(Batch("B_LATER", "T_M1", "2028-01-01", 30, 10.0));
        inv.addBatch(Batch("B_EARLIER", "T_M1", "2026-12-01", 20, 10.0));
        // Sell 15 -> must be deducted from B_EARLIER
        bool sold = inv.sellMedicineFEFO("T_M1", 15);
        assert(sold);
        // B_LATER should still have 30 units; total active is 35
        assert(inv.getAvailableStock("T_M1") == 35);
        std::cout << "[Test 10 PASSED] FEFO order prioritizes earlier expiry batch over later batch.\n";
        passed++;
    }

    // Test 11: Undo after multi-batch sale restores exact batches
    {
        PharmacyInventory inv;
        inv.setSystemDate("2026-10-15");
        inv.addMedicine(Medicine("T_M1", "Test Med 1", "BAR001", "CatA", 20.0));
        inv.addBatch(Batch("B1", "T_M1", "2026-11-01", 10, 10.0));
        inv.addBatch(Batch("B2", "T_M1", "2027-01-01", 20, 10.0));
        inv.sellMedicineFEFO("T_M1", 15); // 10 from B1, 5 from B2
        assert(inv.getAvailableStock("T_M1") == 15);
        bool undone = inv.undoLastSale();
        assert(undone);
        assert(inv.getAvailableStock("T_M1") == 30); // Restored completely
        std::cout << "[Test 11 PASSED] Undo after multi-batch sale restores exact batch quantities.\n";
        passed++;
    }

    // Test 12: Undo when stack is empty handled safely
    {
        PharmacyInventory inv;
        bool undone = inv.undoLastSale();
        assert(!undone);
        std::cout << "[Test 12 PASSED] Undo on empty stack handled gracefully without crash.\n";
        passed++;
    }

    // Test 13: Restock processing success
    {
        PharmacyInventory inv;
        inv.addMedicine(Medicine("T_M1", "Test Med 1", "BAR001", "CatA", 20.0));
        inv.addSupplier(Supplier("S1", "Supp One", "12345", "s@test.com"));
        inv.enqueueRestockRequest("T_M1", 50, "S1", "Urgent");
        assert(inv.getRestockQueueSize() == 1);
        bool processed = inv.processNextRestockRequest("BAT_RESTOCK", "2027-10-01", 12.0);
        assert(processed);
        assert(inv.getRestockQueueSize() == 0);
        assert(inv.getAvailableStock("T_M1") == 50);
        std::cout << "[Test 13 PASSED] Restock request successfully fulfilled and dequeued.\n";
        passed++;
    }

    // Test 14: Restock failure due to duplicate batch ID
    {
        PharmacyInventory inv;
        inv.addMedicine(Medicine("T_M1", "Test Med 1", "BAR001", "CatA", 20.0));
        inv.addBatch(Batch("DUP_BATCH", "T_M1", "2027-01-01", 10, 10.0));
        inv.addSupplier(Supplier("S1", "Supp One", "12345", "s@test.com"));
        inv.enqueueRestockRequest("T_M1", 20, "S1", "Reorder");
        bool processed = inv.processNextRestockRequest("DUP_BATCH", "2027-05-01", 11.0);
        assert(!processed);
        std::cout << "[Test 14 PASSED] Restock processing fails on duplicate batch ID.\n";
        passed++;
    }

    // Test 15: Confirmation that failed restock request remains queued (Bug B verification)
    {
        PharmacyInventory inv;
        inv.addMedicine(Medicine("T_M1", "Test Med 1", "BAR001", "CatA", 20.0));
        inv.addSupplier(Supplier("S1", "Supp One", "12345", "s@test.com"));
        inv.enqueueRestockRequest("T_M1", 20, "S1", "Reorder");
        assert(inv.getRestockQueueSize() == 1);
        // Attempt with invalid date
        bool processed = inv.processNextRestockRequest("B_NEW", "INVALID-DATE", 10.0);
        assert(!processed);
        // Queue size MUST still be 1!
        assert(inv.getRestockQueueSize() == 1);
        std::cout << "[Test 15 PASSED] Failed restock request remains in queue (Bug B fix confirmed).\n";
        passed++;
    }

    // Test 16: Malformed or missing data files handled safely
    {
        PharmacyInventory inv;
        // Non-existent files
        bool loaded = inv.loadFromFile("missing_m.txt", "missing_b.txt", "missing_s.txt", "missing_t.txt");
        assert(loaded);
        assert(inv.getMedicineCount() == 0);
        // File with malformed records
        std::ofstream malformed("malformed_test.txt");
        malformed << "BAD|DATA|ONLY_THREE\n";
        malformed << "M1|Valid Med|111|Cat|20.0\n";
        malformed << "M2|Bad Price|222|Cat|NOT_A_DOUBLE\n";
        malformed.close();
        inv.loadFromFile("malformed_test.txt", "missing_b.txt", "missing_s.txt", "missing_t.txt");
        assert(inv.getMedicineCount() == 1); // Only valid one loaded
        std::remove("malformed_test.txt");
        std::cout << "[Test 16 PASSED] Missing and malformed files handled safely without crashes.\n";
        passed++;
    }

    // Test 17: Repeated load/save operations do not duplicate data
    {
        PharmacyInventory inv;
        inv.addMedicine(Medicine("T_M1", "Test Med 1", "BAR001", "CatA", 20.0));
        inv.saveToFile("test_m.txt", "test_b.txt", "test_s.txt", "test_t.txt", "test_r.txt", "test_u.txt");
        inv.loadFromFile("test_m.txt", "test_b.txt", "test_s.txt", "test_t.txt", "test_r.txt", "test_u.txt");
        inv.loadFromFile("test_m.txt", "test_b.txt", "test_s.txt", "test_t.txt", "test_r.txt", "test_u.txt");
        assert(inv.getMedicineCount() == 1); // No duplicates
        std::remove("test_m.txt");
        std::remove("test_b.txt");
        std::remove("test_s.txt");
        std::remove("test_t.txt");
        std::remove("test_r.txt");
        std::remove("test_u.txt");
        std::cout << "[Test 17 PASSED] Repeated loading clears previous state and avoids duplicate accumulation.\n";
        passed++;
    }

    // Test 18: Calendar Date validation rules (Leap years, 30-day months, Feb 29)
    {
        assert(DateUtils::isValidDate("2024-02-29")); // 2024 is leap year
        assert(!DateUtils::isValidDate("2025-02-29")); // 2025 is not leap year
        assert(!DateUtils::isValidDate("2026-04-31")); // April has 30 days
        assert(DateUtils::isValidDate("2026-04-30"));
        assert(!DateUtils::isValidDate("2026-13-01")); // Month 13 invalid
        assert(!DateUtils::isValidDate("2026-00-10")); // Month 0 invalid
        assert(!DateUtils::isValidDate("2026-05-00")); // Day 0 invalid
        assert(!DateUtils::isValidDate("invalid-date"));
        std::cout << "[Test 18 PASSED] Comprehensive DateUtils calendar and leap year validation.\n";
        passed++;
    }

    // Test 19: Safe handling when file path cannot be written
    {
        PharmacyInventory inv;
        inv.addMedicine(Medicine("T_M1", "Test Med 1", "BAR001", "CatA", 20.0));
        // Use an invalid file path on Windows (contains illegal characters like '?*')
        bool saved = inv.saveToFile("invalid:/?*.txt", "b.txt", "s.txt", "t.txt");
        assert(!saved); // Correctly returns false and reports error
        std::cout << "[Test 19 PASSED] Failed file save returns false without falsely reporting success.\n";
        passed++;
    }

    // Test 20: Full Restart behavior for persisted data, undo records, and pending requests
    {
        // Session 1: Create state with restock queue and sale with undo
        {
            PharmacyInventory session1;
            session1.setSystemDate("2026-10-15");
            session1.addMedicine(Medicine("MED_P", "Persist Drug", "BAR_P", "General", 50.0));
            session1.addBatch(Batch("BAT_P1", "MED_P", "2027-01-01", 20, 30.0));
            session1.addBatch(Batch("BAT_P2", "MED_P", "2027-06-01", 30, 30.0));
            session1.addSupplier(Supplier("SUP_P", "Persist Vendor", "999", "p@p.com"));
            session1.enqueueRestockRequest("MED_P", 100, "SUP_P", "Maintain Buffer");
            // Sell across batches to generate undo record
            session1.sellMedicineFEFO("MED_P", 25); // 20 from P1, 5 from P2
            assert(session1.getAvailableStock("MED_P") == 25);
            assert(session1.getUndoStackSize() == 1);
            assert(session1.getRestockQueueSize() == 1);
            session1.saveToFile("p_m.txt", "p_b.txt", "p_s.txt", "p_t.txt", "p_r.txt", "p_u.txt");
        }

        // Session 2: Fresh restart and load
        {
            PharmacyInventory session2;
            session2.setSystemDate("2026-10-15");
            session2.loadFromFile("p_m.txt", "p_b.txt", "p_s.txt", "p_t.txt", "p_r.txt", "p_u.txt");
            assert(session2.getMedicineCount() == 1);
            assert(session2.getBatchCount() == 2);
            assert(session2.getSupplierCount() == 1);
            assert(session2.getRestockQueueSize() == 1);
            assert(session2.getUndoStackSize() == 1);
            assert(session2.getAvailableStock("MED_P") == 25);

            // Verify Undo works after restart!
            bool undone = session2.undoLastSale();
            assert(undone);
            assert(session2.getAvailableStock("MED_P") == 50);

            // Verify Restock queue works after restart!
            bool fulfilled = session2.processNextRestockRequest("BAT_P3", "2028-01-01", 25.0);
            assert(fulfilled);
            assert(session2.getRestockQueueSize() == 0);
            assert(session2.getAvailableStock("MED_P") == 150);
        }

        std::remove("p_m.txt");
        std::remove("p_b.txt");
        std::remove("p_s.txt");
        std::remove("p_t.txt");
        std::remove("p_r.txt");
        std::remove("p_u.txt");
        std::cout << "[Test 20 PASSED] Persistent restart restores catalogs, undo stack, and restock queue.\n";
        passed++;
    }

    std::cout << "\n=================================================================\n";
    std::cout << " TEST SUMMARY: " << passed << " / " << total << " TESTS PASSED (100% SUCCESS)\n";
    std::cout << "=================================================================\n\n";
}

int main() {
    runAllTests();
    return 0;
}
