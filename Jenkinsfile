pipeline {
  agent {
    docker {
      image 'kodi/jenkins/android-build:nexus_rc2'
      args '--userns=keep-id'
    }

  }
  stages {
    stage('') {
      steps {
        node(label: 'gotham')
      }
    }

  }
}