# ArithLang - LLVM 기반 수식 컴파일러

효과적인 AI Agent 활용 방법을 탐색하기 위한 실험 프로젝트입니다. [HOWTO-AGENT.md](HOWTO-AGENT.md) 문서를 참조하여 AI Agent와의 협업 방법을 확인할 수 있습니다.

AI 개발 시 체계적인 테스트 전략에 대한 상세한 가이드는 [HOWTO-TEST.md](HOWTO-TEST.md) 문서를 참조하세요.

ArithLang은 LLVM을 사용하여 산술 표현식을 실행 가능한 코드로 변환하는 간단한 컴파일러입니다.

## 기능

- 기본 산술 연산: +, -, *, /
- 비교 연산: >, <, >=, <=, ==, !=
- 괄호를 이용한 연산 우선순위 제어
- 부동소수점 숫자 지원 (15자리 고정밀도)
- 변수 할당 및 사용 (x = 1)
- **고급 Print 문 지원**:
  - 문자열 리터럴: `print "Hello, World!";`
  - C-style 포맷 문자열: `print "Value: %.2f", x;`
  - 다중 인수: `print "x=%f, y=%f", a, b;`
  - 이스케이프 시퀀스: `\n`, `\t`, `\"`, `\\`
  - 포맷 지정자: `%f`, `%.Nf`, `%g`, `%e`, `%d`, `%s`, `%%`
- 조건문 지원 (if-else)
- 반복문 지원 (while)
- 라인 주석 지원 (`// comment`)
- LLVM IR 코드 생성
- IR 실행 지원

## Syntax

[Syntax](specs/syntax.md) 문서를 참조하여 ArithLang의 문법을 확인할 수 있습니다.
[Print Statement Specification](specs/print.md) 문서에서 고급 printf-like 출력 기능의 상세한 명세를 확인할 수 있습니다.

### 지원되는 문법 요소

- **산술 연산**: `+`, `-`, `*`, `/` (우선순위 지원)
- **비교 연산**: `>`, `<`, `>=`, `<=`, `==`, `!=`
- **변수**: 할당 및 참조 (`x = 42`)
- **출력**: 고급 `print` 문 
  - 기본: `print x;` (15자리 정밀도, 자동 개행)
  - 문자열: `print "Hello";` (개행 없음)
  - 포맷: `print "Value: %.2f", x;` (개행 없음)
- **주석**: 라인 주석 (`// comment text`)
- **조건문**: `if-else` 구문
  ```
  if (condition) {
      statements
  } else {
      statements
  }
  ```
- **반복문**: `while` 루프
  ```
  while (condition) {
      statements
  }
  ```

## 디렉토리 구조

```
arith_lang/
├── AGENTS.md                        # 모든 AI 에이전트 통합 instruction 파일
├── CLAUDE.md                        # Claude Code용 프로젝트 가이드라인
├── CMakeLists.txt                   # CMake 빌드 설정
├── README.md                        # 프로젝트 문서
├── test_runner.sh                   # 통합 테스트 실행 스크립트
├── .github/                         # GitHub 관련 설정 (copilot-instructions.md)
├── .clinerules/                     # Cline AI 설정 (cline-instructions.md)
├── memory-bank/                     # AI 에이전트 메모리 뱅크 시스템
├── rules/                           # 코드 품질 및 작업 규칙
├── specs/                           # 언어 명세 문서
├── build/                           # CMake 빌드 디렉토리
├── include/                         # 헤더 파일
├── src/                             # 소스 파일
└── tests/                           # 테스트 파일
```

## 아키텍처

ArithLang의 상세한 아키텍처 정보는 [memory-bank/systemPatterns.md](memory-bank/systemPatterns.md)에서 확인하실 수 있습니다.

## 의존성

- LLVM 16+ (개발 헤더 포함)
- CMake 3.16+
- C++17 호환 컴파일러

### macOS에서 LLVM 설치

```bash
brew install llvm
export LLVM_DIR=$(brew --prefix llvm)/lib/cmake/llvm
```

### Ubuntu/Debian에서 LLVM 설치

```bash
sudo apt-get install llvm-dev libllvm-dev
```

## 빌드 방법

```bash
mkdir build
cd build
cmake ..
make
```

## 테스트 실행

### 자동화된 통합 테스트

프로젝트에는 `.k` 소스 파일용 자동화된 테스트 러너가 포함되어 있습니다:

```bash
# 모든 통합 테스트 실행
./test_runner.sh
```

**테스트 러너 기능:**
- `tests/k/` 디렉토리의 모든 `.k` 파일을 컴파일
- 생성된 LLVM IR을 `lli`로 실행
- `// EXPECTED:` 주석에 지정된 예상 결과와 비교
- 색상으로 구분된 통과/실패 결과 및 통계 제공

**테스트 파일 형식:**
```k
// EXPECTED: 120.000000
n = 5;
result = 1;
while (n > 0) {
    result = result * n;
    n = n - 1;
}
print result;
```

### 단위 테스트

Google Test를 사용한 단위 테스트도 포함되어 있습니다:

```bash
# 단위 테스트 빌드 및 실행
cd build
make
ctest
```

## 사용법

ArithLang은 `.k` 확장자를 사용하는 소스 파일을 읽어 LLVM IR을 생성합니다.

### 명령행 형식
```bash
./arithc -o <출력파일> <입력파일>
```

### 소스 파일 작성 (.k 파일)
```bash
# example.k 파일 생성
echo "x = 42; y = x * 2 + 10; print y;" > example.k
```

### 컴파일 및 실행
```bash
# LLVM IR 생성
./arithc -o output.ll example.k

# 생성된 IR 실행
lli output.ll
```

## 예제

다양한 ArithLang 예제는 `tests/k/` 디렉토리에서 확인하실 수 있습니다:

### 기본 산술 연산
```bash
# 팩토리얼 계산 예제 실행
$ ./arithc -o factorial.ll tests/k/factorial.k
$ lli factorial.ll
120.000000
```

### 비교 연산자
```bash
# 비교 연산 예제 실행
$ ./arithc -o comparison.ll tests/k/test_comparison.k
$ lli comparison.ll
1.000000
```

### 조건문과 반복문
```bash
# while 루프 예제 실행
$ ./arithc -o while.ll tests/k/test_while.k
$ lli while.ll
3.000000
2.000000
1.000000
```

### 연산자 우선순위
```bash
# 우선순위 테스트 예제 실행
$ ./arithc -o precedence.ll tests/k/test_precedence.k
$ lli precedence.ll
1.000000
```

### 고급 Printf 기능
```bash
# printf 기능 종합 테스트 실행
$ ./arithc -o printf.ll tests/k/test_complete_printf.k
$ lli printf.ll
Hello, ArithLang!This line has
a newlinePi = 3.142
Scientific: 3.141593e+00
x=10.500000, y=20.300000, sum=30.8
Language: ArithLang
Progress: % complete
```

### 사용 가능한 예제 파일들

- **기본 연산**: `factorial.k`, `hello.k`, `pi_leibniz.k`
- **비교 연산**: `test_comparison.k`, `test_eq.k`, `test_gte.k`, `test_lt.k`, `test_lte.k`, `test_neq.k`
- **제어 구조**: `test_if.k`, `test_if_false.k`, `test_while.k`
- **기타**: `test_precedence.k`, `test_unary_minus.k`, `test_complete_printf.k`

## IR 실행 방법

생성된 LLVM IR을 실제로 실행할 수 있습니다:

### 📝 주의사항: main 함수에 대해

ArithLang이 생성하는 `main` 함수는 표준 C/C++의 `main` 함수와 다릅니다:

| 구분 | ArithLang | 표준 C/C++ |
|------|-----------|-------------|
| **시그니처** | `define double @main()` | `define i32 @main(i32 %argc, i8** %argv)` |
| **반환 타입** | `double` (계산 결과) | `int` (종료 코드) |
| **매개변수** | 없음 | `argc`, `argv` |
| **역할** | 표현식 평가 함수 | 프로그램 진입점 |

```llvm
; ArithLang의 main (표현식 평가용)
define double @main() {
entry:
  %addtmp = fadd double 2.000000e+00, 3.000000e+00
  ret double %addtmp
}

; 표준 C/C++의 main (프로그램 진입점)
define i32 @main(i32 %argc, i8** %argv) {
entry:
  ret i32 0
}
```

현재는 **하이브리드 형태**로, LLVM 인터프리터(`lli`)가 관대하게 실행해주지만 엄밀히는 표준적이지 않습니다.

### 방법 1: LLVM 인터프리터 사용
```bash
# 소스 파일 작성
echo "x=1; print x;" > test.k

# 컴파일 및 실행
./arithc -o test.ll test.k && lli test.ll
# 출력: 1.000000
```

### 방법 2: 네이티브 실행파일 컴파일
```bash
# 소스 파일 작성
echo "x=1; print x;" > test.k

# LLVM IR 생성
./arithc -o test.ll test.k

# 어셈블리로 컴파일
llc test.ll -o test.s

# 실행파일로 컴파일
gcc test.s -o test_exec

# 실행
./test_exec
# 출력: 1.000000
```

## 도움말 보기
```bash
# 사용법 안내
./arithc
```
