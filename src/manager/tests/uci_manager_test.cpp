#include <gtest/gtest.h>
#include "manager/uci_manager.h"

// Test if the engine is available
TEST(UCIManagerTest, EngineAvailable)
{
  UCIManager manager;
  ASSERT_TRUE(manager.is_engine_available("stockfish"));
  ASSERT_FALSE(manager.is_engine_available("non_existent_engine"));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
