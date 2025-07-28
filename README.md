# ArithLang - LLVM 기반 수식 컴파일러

LLVM을 사용하여 산술 표현식을 실행 가능한 코드로 변환하는 간단한 컴파일러입니다.

## 기능

- 기본 산술 연산: +, -, *, /
- 비교 연산: >, <, >=, <=, ==, !=
- 괄호를 이용한 연산 우선순위 제어
- 부동소수점 숫자 지원
- 변수 할당 및 사용 (x = 1)
- Print 문 지원 (print x)
- 조건문 지원 (if-else)
- 반복문 지원 (while)
- 라인 주석 지원 (`// comment`)
- LLVM IR 코드 생성
- IR 실행 지원

## Syntax

[Syntax](syntax.md) 문서를 참조하여 ArithLang의 문법을 확인할 수 있습니다.

### 지원되는 문법 요소

- **산술 연산**: `+`, `-`, `*`, `/` (우선순위 지원)
- **비교 연산**: `>`, `<`, `>=`, `<=`, `==`, `!=`
- **변수**: 할당 및 참조 (`x = 42`)
- **출력**: `print` 문 (`print x;`)
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

## 아키텍처

1. **Lexer** (`src/lexer.cpp`): 입력 문자열을 토큰으로 분해
   - 산술/비교 연산자, 키워드(`if`, `else`, `while`, `print`), 식별자, 숫자 인식
   - 다중 문자 연산자 지원 (`>=`, `<=`, `==`, `!=`)
   - 라인 주석 처리 (`//`)

2. **Parser** (`src/parser.cpp`): 토큰을 추상 구문 트리(AST)로 변환
   - 연산자 우선순위 파싱 (Pratt parser 스타일)
   - 우선순위: 비교연산(5) < 덧셈/뺄셈(10) < 곱셈/나눗셈(40)
   - 조건문, 반복문, 블록 구문 지원

3. **AST** (`include/ast.h`): 추상 구문 트리 노드들
   - 표현식: `NumberExprAST`, `VariableExprAST`, `BinaryExprAST`, `AssignmentExprAST`
   - 문장: `PrintStmtAST`, `IfStmtAST`, `WhileStmtAST`, `BlockAST`

4. **CodeGen** (`src/codegen.cpp`): AST를 LLVM IR로 변환
   - 변수 저장을 위한 alloca/load/store 명령어 생성
   - 조건/반복문을 위한 기본 블록 및 분기 명령어 생성
   - 비교 연산을 위한 LLVM 부동소수점 비교 명령어

5. **Main** (`src/main.cpp`): 컴파일러 드라이버

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
make test_syntax
./test_syntax
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

### 기본 산술 연산
```bash
# basic.k 파일 생성
echo "2 + 3 * 4" > basic.k

# 컴파일
$ ./arithc -o basic.ll basic.k
LLVM IR이 성공적으로 생성되었습니다: basic.ll

# 생성된 IR 확인
$ cat basic.ll
; ModuleID = 'ArithLang'
define double @main() {
entry:
  %multmp = fmul double 3.000000e+00, 4.000000e+00
  %addtmp = fadd double 2.000000e+00, %multmp
  ret double %addtmp
}
```

### 변수 할당과 출력
```bash
# program.k 파일 생성
echo "x=42; y=x*2+10; print y;" > program.k

# 컴파일 및 실행
$ ./arithc -o program.ll program.k
LLVM IR이 성공적으로 생성되었습니다: program.ll

$ lli program.ll
94.000000
```

### 비교 연산자
```bash
# comparison.k 파일 생성
echo "x = 5; y = 3; if (x > y) { print 1; } else { print 0; }" > comparison.k

# 컴파일 및 실행
$ ./arithc -o comparison.ll comparison.k
LLVM IR이 성공적으로 생성되었습니다: comparison.ll

$ lli comparison.ll
1.000000
```

### 조건문과 반복문
```bash
# loop.k 파일 생성
echo "x = 3; while (x) { print x; x = x - 1; }" > loop.k

# 컴파일 및 실행
$ ./arithc -o loop.ll loop.k
LLVM IR이 성공적으로 생성되었습니다: loop.ll

$ lli loop.ll
3.000000
2.000000
1.000000
```

### 연산자 우선순위
```bash
# precedence.k 파일 생성
echo "x = 2 + 3 > 4; if (x) { print 1; } else { print 0; }" > precedence.k

# 컴파일 및 실행
$ ./arithc -o precedence.ll precedence.k
LLVM IR이 성공적으로 생성되었습니다: precedence.ll

$ lli precedence.ll
1.000000
```

### 중첩된 조건문
```bash
# nested.k 파일 생성
echo "x = 10; if (x > 5) { if (x < 15) { print 1; } else { print 2; } } else { print 0; }" > nested.k

# 컴파일 및 실행
$ ./arithc -o nested.ll nested.k
LLVM IR이 성공적으로 생성되었습니다: nested.ll

$ lli nested.ll
1.000000
```

### 팩토리얼 계산 (반복문 활용)
```bash
# factorial.k 파일 생성
echo "n = 5; result = 1; while (n > 0) { result = result * n; n = n - 1; } print result;" > factorial.k

# 컴파일 및 실행
$ ./arithc -o factorial.ll factorial.k
LLVM IR이 성공적으로 생성되었습니다: factorial.ll

$ lli factorial.ll
120.000000
```

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

### 도움말 보기
```bash
# 사용법 안내
./arithc
```
