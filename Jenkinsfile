pipeline {
    options {
        buildDiscarder(logRotator(daysToKeepStr: '7', numToKeepStr: '35', artifactNumToKeepStr: '20'))
    }

    parameters {
        //choice(name: 'Configuration', choices: [Default, Debug, Release], description: 'Default is decided by the in tree build scripts (see tools/buildsteps/defaultenv).')
        string(name: 'Revision', defaultValue: 'master', description: 'master')
        //booleanParam(name: 'UPLOAD_RESULT', defaultValue: true, description: 'Whether the resulting builds should be uploaded to test-builds')
        string(name: 'GITHUB_REPO', defaultValue: 'xbmc', description: 'The repository/fork to build from. (e.x. the name of the github user).')
        //booleanParam(name: 'RUN_TEST', defaultValue: false, description: 'Turn this on if you want to build and run the xbmc unit tests based on gtest.')
        //booleanParam(name: 'BUILD_OBB', defaultValue: false, description: 'Enable this if you want to split the resulting android package into apk (< 50MB) and obb package (needed for satisfying size restriction on google play store). Do not use this in normal situations.')
        //booleanParam(name: 'BUILD_BINARY_ADDONS', defaultValue: true, description: 'Whether binary addons should be built during or not.')
        //string(name: 'ADDONS', defaultValue: '^peripheral\\.joystick$', description: 'Which binary addons should be built.')
        //choice(name: 'ANDROIDSTORE', choices: [none, GOOGLE], description: 'which android store to build for. Google disables certain permissions')
    }

    environment {
        BUILDTHREADS = 64
        CCACHE_DIR = '$WORKSPACE/.ccache'
        ndkver = "21.4.7075529"
    }

    agent {
        docker {
            image 'kodi/jenkins/android-build:nexus_rc2'
            label 'gotham'
            args '--userns=keep-id'
            customWorkspace "workspace/$JOB_BASE_NAME-Nexus"
        }
    } 
    stages {
        stage('Checkout Scm') {
            steps {
                git(
                    //branch: '+refs/heads/*:refs/remotes/origin/* +refs/heads/*:refs/remotes/params.${GITHUB_REPO}/* +refs/pull/params.${ghprbPullId}/*:refs/remotes/origin/pr/${ghprbPullId}/* +refs/pull/${ghprbPullId}/*:refs/remotes/params.${GITHUB_REPO}/pr/${ghprbPullId}/*',
                    branch: params.Revision,
                    credentialsId: 'github-app-xbmc', url: 'https://github.com/' + params.GITHUB_REPO + '/xbmc.git'
                )
            }
            
        }

        stage('Build') {
            steps {
                script {
                    sh 'bash -c "\
                      git clean -xfd \
                      && . $WORKSPACE/tools/buildsteps/android-arm64-v8a/prepare-depends \
                      && cd $WORKSPACE/tools/depends \
                      && ./configure \
                        --with-tarballs=$WORKSPACE/tools/depends/xbmc-depends/xbmc-tarballs \
                        --host=aarch64-linux-android \
                        --with-sdk-path=/home/jenkins/android-tools/android-sdk-linux/cmdline-tools \
                        --with-ndk-path=/home/jenkins/android-tools/android-sdk-linux/cmdline-tools/ndk/$ndkver \
                        --prefix=$WORKSPACE/tools/depends/xbmc-depends \
                       && make -j$BUILDTHREADS
                    "'

                    sh '\
                      cd $WORKSPACE \
                      && rm -rf $WORKSPACE/build \
                      && make -C $WORKSPACE/tools/depends/target/cmakebuildsys \
                      && cd build \
                      && make -j$BUILDTHREADS VERBOSE=1 \
                      && make -j$BUILDTHREADS apk \
    				        '
                }
            }
        }
    }
}
