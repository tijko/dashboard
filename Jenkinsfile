pipeline {
    agent any
    stages {
        stage('Build') {
            steps {
                sh " sudo apt-get install -y libprocps-dev"
                sh " sudo apt-get install -y ncurses-dev"
                sh " sudo make"
            }
        }
        stage('Deploy') {
            steps {
                sh " sudo make install"
                sh " make clean"
            }
        }
    }
}
