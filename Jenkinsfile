pipeline {
    agent any
    stages {
        stage('Build') {
            steps {
                sh "sudo apt-get install libprocps-dev"
                sh "sudo apt-get install ncurses-dev"
                sh "sudo make"
            }
        }
        stage('Deploy') {
            steps {
                sh "sudo make install"
                sh "make clean"
            }
        }
    }
}
