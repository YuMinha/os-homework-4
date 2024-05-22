# Thread pool을 사용하여 Gaussian Elimination 방식으로 연립방정식 풀기
This example project is written in C, and tested with make and bash scripts.

### The assignment
- $n \times n$ 행렬 이진 파일과 $n$ 벡터 이진 파일에서 double 크기의 데이터를 읽어 행렬 A와 벡터 B에 저장한 뒤, $AX = B$를 푸는 $n$ 벡터 $X$를 구해 $n$ 벡터 이진 파일에 저장하는 프로그램을 완성한다. 프로그램에서 이 세 파일의 이름은 인자로 받으며 argv[1]과 argv[2]는 입력으로 각각 $n \times n$ 행렬 이진 파일명과 $n$ 벡터 이진 파일명에 해당하고 배열 $A$와 $B$에 로드된다. argv[3]은 답 벡터 $X$를 저장할 파일 이름이다. argv[4]는 thread pool의 크기이며 작업을 수행할 thread의 개수와도 같다. 행렬 $A$는 일차원 배열 또는 이차원 배열과 같이 자유롭게 구성할 수 있다. 현재 부분적으로 실행되는 프로그램이 hw4.c로 제공되는데 이는 과제 1에 제공되는 hw1.c에서 시간과 병렬성 측정을 위한 timelog 함수들이 추가된 코드이다.

- 연립방정식을 풀기 위한 과정은 과제1의 명세서를 참고한다.
- POSIX API의 일부를 wrapper 함수의 형태로 wrapper.c에서 제공하므로, 가급적 제공되는 이 함수들을 이용한다.
- Thread pool은 리눅스나 POSIX에서 따로 제공되는 API가 아니고 단지 여러 thread를 사용하는 설계상의 개념이므로 thread pool을 만드는 것은 Pthread_create()를 이용하여 작업 스레드를 생성하는 것과 동일하다. Thread pool과 과제 3의 multi-thread의 차이는 다음과 같다.
  + Thread pool은 시작할 때 자신의 담당 영역이 정해지지 않는다. 
  + Thread pool은 bounded buffer에 작업이 있으면 이를 가져와서 실행하는 일을 반복한다.
  + Main thread가 모든 작업을 bounded buffer에 제출하면 thread pool을 비우는 작업을 시작한다. 
    * 이 작업은 bounded buffer에 sentinel value들을 thread pool의 thread 개수만큼 제출하는 것이다.
	* Sentinel value를 받은 thread는 루프를 벗어나 종료한다.
- 작업 스레드들은 전역 변수를 공유하기 때문에 과제2에서 사용했던 공유메모리는 필요하지 않다. 
- Gaussian elimination은 한 단계의 작업이 모두 끝나야 다음 단계로 넘어갈 수 있다는 특징이 있으며 이는 thread pool을 사용하더라도 여전히 살아있는 특징이다. 하지만, thread pool에서는 모든 작업들이 같이 시작하는 것이 아니기 때문에 barrier를 사용할 수 없다. 대신, 다음과 같이 한 단계의 작업을 동기화시킨다.
  + Main thread는 한 단계에서 n개의 작업을 bounded buffer에 제출할 예정이면 1-n의 값으로 semaphore를 초기화한다.
  + Main thread는 n개의 작업을 제출하고 Sem_wait()를 수행한다.
  + Thread pool의 worker thread는 작업을 받아 수행하고 Sem_post()를 수행한다.
  + Worker thread가 n개의 Sem_post()를 수행해야 semaphore 값이 1이 되어 main thread는 Sem_wait()에서 통과한다. 
  + POSIX에서 제공하는 semaphore는 음수 값을 허용하지 않기 때문에 wrapper.c에서 제공하는 mutex와 condition variable로 구현된 Semaphore를 사용해야 한다.

- 프로그램을 모두 작성했거나 수정했으면 다음과 같이 컴파일한다.

`make all`

- 제대로 컴파일되면 hw4, hwdiff, showdata 실행 파일이 만들어진다. hw4는 다음과 같이 실행할 수 있다. 여기서 thread pool의 크기를 3으로 생성시켰다.

`./hw4 A2.dat B2.dat X2.dat 3`

- 프로그램이 정상적으로 종료되면 쉘에게 0을 반환하고 이진 파일 X2.dat를 생성한다. hw4가 4개의 인자를 받지 못하면 다음 에러 메시지를 표준에러로 출력하며 1을 반환한다.

`Usage: ./hw4 <mat> <invec> <outvec> <np>`

- 이진 행렬 A2.dat의 내용을 확인하려면 첫 번째 인자로 m을 넣어 다음과 같이 실행한다.

`./showdata m A2.dat`

- 이진 벡터 X2.dat의 내용을 확인하려면 파일 이름을 인자로 넣어 다음과 같이 실행한다.

`./showdata X2.dat`

- showdata로 확인하면 A2.dat와 B2.dat를 입력으로 받아 구한 연립방정식의 해 X2.dat는 같이 제공되는 파일 X2a.dat와 같다. 하지만, 다음과 같이 Linux의 diff로 동일 내용인지 체크하면 다르다고 나온다.

`diff X.dat Xa.dat`

`Binary files X2.dat and X2a.dat differ`

- 이는 실수 연산 (floating point operation)의 round-off로 인해 발생하는 아주 작은 값의 오차 때문이다. 이 문제 때문에 결과 파일과 정답을 비교하기 위해 diff 대신 hwdiff를 다음과 같이 사용한다. diff와 마찬가지로 hwdiff는 두 파일의 내용이 거의 같을 때 아무런 메시지를 출력하지 않으며 쉘에 0을 반환한다. 여기서 세 번째 인자로 어느 정도까지의 오차를 인정하는지 입력한다. 예를 들어, 두 값의 차이가 0.0001보다 작다고 인정할 경우에는 다음과 같이 실행한다.

`./hwdiff X2.dat X2a.dat 0.0001`

- diff와 마찬가지로 hwdiff는 실행결과 아무런 메시지가 출력되지 않으면 두 파일이 같다는 것을 의미한다.

### Setup command
N/A

### Run command to test program
- GitHub에서 과제가 제대로 수행되는지 테스트를 할 때는 아래의 명령을 사용한다. 과제를 최종 제출하기 전에 자신의 로컬 리눅스 컴퓨터에서 이 명령을 실행하여 제대로 수행되는지 확인할 수 있다. 이를 위해서 먼저 이 repository의 파일들을 모두 다운로드 받아 다음을 순서대로 실행하면 된다.

`make test ARG=1`

`make test ARG=2`

`make test ARG=3`

`make test ARG=4`

`make test ARG=5`

`make test ARG=6`

`make test ARG=7`

`make test ARG=8`

`make test ARG=9`

### Notes
- `gcc` can be used to compile and link C applications for use with existing test harnesses or C testing frameworks.
