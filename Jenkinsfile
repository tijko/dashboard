pipeline {
    agent any
    stages {
        stage('Build') {
            steps {
                sh 'apt install -y libprocps-dev'
                sh 'apt install -y ncurses-dev'
                sh 'make'
            }
        }
        stage('Deploy') {
            steps {
                sh 'make install'
            }
        }
    }
}
