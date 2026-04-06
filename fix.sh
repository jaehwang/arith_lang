#!/bin/bash
sed -i '337,374c\
    // Program: global scope\
    if (auto program = dynamic_cast<ProgramAST*>(node)) {\
        env.enterScope();\
        for (const auto& exp : program->getExports()) {\
            if (exp->getDeclaration()) {\
                typeCheckNode(exp->getDeclaration(), env, filename);\
            }\
        }\
        for (const auto& stmt : program->getStatements()) {\
            typeCheckNode(stmt.get(), env, filename);\
        }\
        env.exitScope();\
        return;\
    }\
}\
} // namespace\
' src/type_check.cpp
