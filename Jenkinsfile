pipeline {
    agent any
    stages {
        stage('Build') {
            steps {
                sh "apt-get install -y libprocps-dev"
                sh "apt-get install -y ncurses-dev"
                sh "make"
            }
        }
        stage('Deploy') {
            steps {
                sh "make install"
            }
        }
    }
}
