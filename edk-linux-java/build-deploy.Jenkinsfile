node("edk_linux_java_builder") {
    timestamps {
        stage("checkout") {
            git url: "hueivaAtDtfBitBucket:/hent/edk.git", branch: "master"
        }
        
        stage("build") {
            sh label: "build", script: "rm -rf build/ && mkdir build/ && cd build/ && cmake -DBUILD_WRAPPERS=ON .. && make"
        }

        dir("edk-linux-java") {
            stage("deploy") {
                sh label: "deploy", script: "./deploy-wrappers.sh"
            }
        }
    }
}
