mvn deploy:deploy-file -B -Durl=https://artifactory-ehv.rndit.intra.lighting.com/artifactory/hue_system/ \
                       -DrepositoryId=snapshots \
                       -DgeneratePom=true \
                       -Dversion=1.32-$(git show -s --format=%h) \
                       -DgroupId=com.signify.hue.edk.libs \
                       -DartifactId=edk-linux-java \
                       -Dfile=../build/bin/libhuestream_java_managed.jar \
                       -Dfiles=../build/bin/libhuestream_java_native.so \
                       -Dtypes=so \
                       -Dclassifiers=native_library