import ecss_smp
import xsmp
import project_name

class Testproject_name(xsmp.unittest.TestCase):
    try:
        sim: project_name._test_project_name.Simulator
    except AttributeError:
        pass
    
    def loadAssembly(self, sim: ecss_smp.Smp.ISimulator):
        sim.LoadLibrary("project_name")
        # Create instances, configuration, and connections here.

    def test_project_name(self):
        # Add project-specific assertions here.
        pass
