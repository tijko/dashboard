pipeline {
    agent any
    stages {
        stage('Build') {
            steps {
                sh "apt-get install libprocps-dev"
                sh "apt-get install ncurses-dev"
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
