#!/usr/bin/env python
from autogen.autogen import autogen

project = "KDTools"
version = "2.3.99"
subprojects = ( "KDToolsCore", "KDToolsGui", "KDUnitTest", "KDUpdater" )
prefixed = False
forwardHeaderMap = { "KDUpdater/KDUpdater":"kdupdater.h" }
autogen(project, version, subprojects, prefixed, forwardHeaderMap = forwardHeaderMap, policyVersion = 2)

