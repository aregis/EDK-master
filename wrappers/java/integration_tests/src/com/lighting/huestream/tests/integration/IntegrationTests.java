package com.lighting.huestream.tests.integration;


import org.junit.runner.RunWith;
import org.junit.runners.Suite;

@RunWith(Suite.class)
@Suite.SuiteClasses({
        AnimationTests.class,
        ConnectivityTests.class
})
public class IntegrationTests {
    // the class remains empty,
    // used only as a holder for the above annotations
}
