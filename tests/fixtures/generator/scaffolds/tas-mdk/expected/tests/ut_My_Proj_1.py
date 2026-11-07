from gram_addons__python_test_suite.gram_test_case import GramTestCase, SECOND

class MyModelTestCase(GramTestCase):
    def buildSimulatorData(self, jsim):
        pass

    def test_model_feature1(self):
        self.sim.run_delta(SECOND)
