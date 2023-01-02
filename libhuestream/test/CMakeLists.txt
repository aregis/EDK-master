
list(APPEND source_files
    huestream/TestHueEDK.cpp
    huestream/TestHueStream.cpp
    huestream/common/TestSerializable.cpp
    huestream/common/TestSerializeBase.cpp
    huestream/common/data/TestApiVersion.cpp
    huestream/common/data/TestArea.cpp
    huestream/common/data/TestBridge.cpp
    huestream/common/data/TestColor.cpp
    huestream/common/data/TestCuboidArea.cpp
    huestream/common/data/TestGroup.cpp
    huestream/common/http/TestBridgeHttpClient.cpp
    huestream/common/language/TestDummyTranslator.cpp
    huestream/common/storage/TestBridgeFileStorageAccessor.cpp
    huestream/common/util/TestHueMath.cpp
    huestream/common/util/TestRand.cpp
    huestream/connect/TestBasicGroupLightController.cpp
    huestream/connect/TestBridgeStreamingChecker.cpp
    huestream/connect/TestConfigRetriever.cpp
    huestream/connect/TestConnect.cpp
    huestream/connect/TestConnectionFlowBase.cpp
    huestream/connect/TestConnectionFlowExistingBridge.cpp
    huestream/connect/TestConnectionFlowManual.cpp
    huestream/connect/TestConnectionFlowNewBridge.cpp
    huestream/connect/TestConnectionMonitor.cpp
    huestream/connect/TestDefaultAuthenticator.cpp
    huestream/connect/TestDefaultConnectionFlowFactory.cpp
    huestream/connect/TestFeedbackMessage.cpp
    huestream/effect/TestLightScript.cpp
    huestream/effect/TestMixer.cpp
    huestream/effect/TestTimeline.cpp
    huestream/effect/animation/TestActionPlayer.cpp
    huestream/effect/animation/TestPlayer.cpp
    huestream/effect/animation/animations/TestConstant.cpp
    huestream/effect/animation/animations/TestCurve.cpp
    huestream/effect/animation/animations/TestRandom.cpp
    huestream/effect/animation/animations/TestSequence.cpp
    huestream/effect/animation/animations/TestTween.cpp
    huestream/effect/animation/animations/TestFrames.cpp
    huestream/effect/animation/data/TestCurveData.cpp
    huestream/effect/animation/data/TestCurveOptions.cpp
    huestream/effect/animation/data/TestVector.cpp
    huestream/effect/effects/TestAreaEffect.cpp
    huestream/effect/effects/TestExplosionEffect.cpp
    huestream/effect/effects/TestLightIteratorEffect.cpp
    huestream/effect/effects/TestLightSourceEffect.cpp
    huestream/effect/effects/TestSphereLightSourceEffect.cpp
    huestream/effect/effects/TestManualEffect.cpp
    huestream/effect/effects/TestMultiChannelEffect.cpp
    huestream/stream/TestDefaultTimerProvider.cpp
    huestream/stream/TestProtocolSerializer.cpp
    huestream/stream/TestStream.cpp
    huestream/stream/TestStreamStarter.cpp
    huestream/_mock/MockAction.h
    huestream/_mock/MockAnimationEffect.h
    huestream/_mock/MockBasicGroupLightController.h
    huestream/_mock/MockBridge.h
    huestream/_mock/MockBridgeAuthenticator.h
    huestream/_mock/MockBridgeHttpClient.h
    huestream/_mock/MockBridgeSearcher.h
    huestream/_mock/MockBridgeStateChecker.h
    huestream/_mock/MockBridgeStorageAccessor.h
    huestream/_mock/MockConfigRetriever.h
    huestream/_mock/MockConnect.h
    huestream/_mock/MockConnectionFlowFactory.h
    huestream/_mock/MockConnectionMonitor.h
    huestream/_mock/MockConnector.h
    huestream/_mock/MockEffect.h
    huestream/_mock/MockFeedbackMessageHandler.h
    huestream/_mock/MockHttpClient.h
    huestream/_mock/MockHueStream.h
    huestream/_mock/MockMessageDispatcher.h
    huestream/_mock/MockMessageTranslator.h
    huestream/_mock/MockMixer.h
    huestream/_mock/MockScheduler.h
    huestream/_mock/MockStream.h
    huestream/_mock/MockStreamFactory.h
    huestream/_mock/MockStreamStarter.h
    huestream/_mock/MockTimeManager.h
    huestream/_mock/MockTimeline.h
    huestream/_stub/StubMessageDispatcher.h
    huestream/_stub/StubTimeProvider.h
    huestream/common/TestSerializeBase.h
    huestream/connect/TestConnectionFlowBase.h
    huestream/connect/TestableConnectionFlow.h
    huestream/effect/effects/TestExplosionEffect.h
)

add_executable(huestream_tests ${source_files})
target_link_libraries(huestream_tests
        huestream
        gtest_main
        gtest
        gmock
        )

if (APPLE)
    target_link_libraries(huestream_tests "-framework SystemConfiguration")
endif()

if (BUILD_TEST_COV)
    set(coverage_project_name huestream_tests_coverage)
    set(coverage_build_dir ${CMAKE_BINARY_DIR}/bin/coverage)
    add_custom_target( ${coverage_project_name}
            DEPENDS huestream_tests
            )

    add_custom_command(TARGET ${coverage_project_name}
            COMMENT "Run tests for coverage"
            POST_BUILD COMMAND $<TARGET_FILE:huestream_tests>
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            )

    add_custom_command(TARGET ${coverage_project_name}
            COMMENT "Creating dir: ${coverage_build_dir}"
            POST_BUILD COMMAND ${CMAKE_COMMAND}
            ARGS -E make_directory ${coverage_build_dir})

    add_custom_command(TARGET ${coverage_project_name}
            COMMENT "Generating html report coverage: ${coverage_build_dir}/${coverage_project_name}.html"
            POST_BUILD COMMAND gcovr
            ARGS --html --html-details -f '.*/huestream/.*' -o ${coverage_build_dir}/${coverage_project_name}.html -r .
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )

    add_custom_command(TARGET ${coverage_project_name}
            COMMENT "Generating xml report coverage: ${coverage_build_dir}/${coverage_project_name}.xml"
            POST_BUILD COMMAND gcovr
            ARGS --xml -f '.*/huestream/.*' -o ${coverage_build_dir}/${coverage_project_name}.xml -r .
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )

    include(ExternalCpplint)
    add_dependencies(huestream_tests ${EXTERNAL_CPPLINT})
    set(LINT_RUNNER_DIR ${CMAKE_CURRENT_LIST_DIR}/../../tools/cpplint)

    add_custom_command(TARGET ${coverage_project_name}
            COMMENT "Launching cpplint runner: ${LINT_RUNNER_DIR}/lint_edk.sh"
            POST_BUILD COMMAND ./lint_edk.sh ${CPPLINT_EXECUTABLE}
            WORKING_DIRECTORY ${LINT_RUNNER_DIR}
            )
endif()


if (WIN32)
    copy_libs_to_output(huestream_tests "${shared_libs}")
endif()
