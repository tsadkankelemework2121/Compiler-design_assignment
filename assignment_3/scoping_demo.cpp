/*
Scoping Demo - Static vs Dynamic Scoping
This program demonstrates the difference between static (lexical) and dynamic scoping.
*/

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <memory>

// Forward declarations
class Environment;
class Function;

// Value type that can hold either an integer or a function pointer
class Value {
public:
    enum Type { INT, FUNC };
    Type type;
    int int_val;
    Function* func_val;
    
    Value() : type(INT), int_val(0), func_val(nullptr) {}
    Value(int v) : type(INT), int_val(v), func_val(nullptr) {}
    Value(Function* f) : type(FUNC), func_val(f) {}
};

// Expression type - can be int literal or variable name
class Expr {
public:
    enum Type { INT_LIT, VAR };
    Type type;
    int int_val;
    std::string var_name;
    
    Expr(int v) : type(INT_LIT), int_val(v) {}
    Expr(const std::string& name) : type(VAR), var_name(name) {}
};

// Statement type
class Statement {
public:
    enum Type { ASSIGN, PRINT, CALL, DEF };
    Type stmt_type;
    std::string name;  // variable name or function name
    Expr expr;  // for assign and print
    std::vector<Statement> body;  // for def
    
    Statement(Type t, const std::string& n, const Expr& e) 
        : stmt_type(t), name(n), expr(e) {}
    Statement(Type t, const std::string& n, const std::vector<Statement>& b) 
        : stmt_type(t), name(n), body(b) {}
    Statement(Type t, const Expr& e) 
        : stmt_type(t), name(""), expr(e) {}
    Statement(Type t, const std::string& n) 
        : stmt_type(t), name(n) {}
};

class Environment {
public:
    // I'm using an unordered_map to store variable bindings
    std::unordered_map<std::string, Value> vars;
    // Parent environment for nested scopes
    Environment* parent;
    
    Environment(Environment* p = nullptr) : parent(p) {}
    
    Value get(const std::string& name) {
        // Look up a variable in this environment or parent
        auto it = vars.find(name);
        if (it != vars.end()) {
            return it->second;
        }
        if (parent) {
            return parent->get(name);
        }
        throw std::runtime_error("Variable '" + name + "' not found");
    }
    
    void set(const std::string& name, const Value& value) {
        // Set a variable in this environment
        vars[name] = value;
    }
};

class Function {
public:
    std::string name;
    std::vector<Statement> body;  // List of statements to execute
    // For static scoping: capture the environment where function is defined
    Environment* definition_env;
    
    Function(const std::string& n, const std::vector<Statement>& b, Environment* def_env = nullptr)
        : name(n), body(b), definition_env(def_env) {}
    
    Value call(Environment* call_env, const std::string& scoping_mode) {
        /*
        Execute the function body.
        scoping_mode: "static" uses definition_env, "dynamic" uses call_env
        */
        Environment* env;
        if (scoping_mode == "static") {
            // Static scoping: use the environment where function was defined
            // This is the key difference - we use definition_env, not call_env
            env = new Environment(definition_env);
        } else {
            // Dynamic scoping: use the environment where function is called
            // This means we look up variables in the call stack
            env = new Environment(call_env);
        }
        
        // Execute function body in the chosen environment
        Value result;
        for (const auto& stmt : body) {
            result = execute_stmt(stmt, env, scoping_mode);
        }
        delete env;
        return result;
    }
};

// Forward declaration
Value execute_expr(const Expr& expr, Environment* env, const std::string& scoping_mode);

Value execute_stmt(const Statement& stmt, Environment* env, const std::string& scoping_mode) {
    // Execute a single statement
    if (stmt.stmt_type == Statement::ASSIGN) {
        // Assignment: x = value
        Value value = execute_expr(stmt.expr, env, scoping_mode);
        env->set(stmt.name, value);
        return value;
    }
    else if (stmt.stmt_type == Statement::PRINT) {
        // Print statement: print(expr)
        Value value = execute_expr(stmt.expr, env, scoping_mode);
        std::cout << value.int_val << std::endl;
        return value;
    }
    else if (stmt.stmt_type == Statement::CALL) {
        // Function call: func_name()
        Value func_val = env->get(stmt.name);
        if (func_val.type != Value::FUNC) {
            throw std::runtime_error("'" + stmt.name + "' is not a function");
        }
        // When calling, pass current environment for dynamic scoping
        return func_val.func_val->call(env, scoping_mode);
    }
    else if (stmt.stmt_type == Statement::DEF) {
        // Function definition: def func_name(): body
        // Create function and capture current environment for static scoping
        Function* func = new Function(stmt.name, stmt.body, env);
        env->set(stmt.name, Value(func));
        return Value(func);
    }
    return Value();
}

Value execute_expr(const Expr& expr, Environment* env, const std::string& scoping_mode) {
    // Evaluate an expression
    if (expr.type == Expr::INT_LIT) {
        // Integer literal
        return Value(expr.int_val);
    }
    else if (expr.type == Expr::VAR) {
        // Variable reference
        return env->get(expr.var_name);
    }
    throw std::runtime_error("Unknown expression type");
}

Environment* run_program(const std::vector<Statement>& program, const std::string& scoping_mode) {
    /*
    Run a program with the specified scoping mode.
    program: vector of statements
    scoping_mode: "static" or "dynamic"
    */
    // Global environment
    Environment* global_env = new Environment();
    
    // Execute all statements
    for (const auto& stmt : program) {
        execute_stmt(stmt, global_env, scoping_mode);
    }
    
    return global_env;
}

// Define the example program
// x = 10
// def f():
//     print(x)
// def g():
//     x = 20
//     f()

std::vector<Statement> example_program = {
    Statement(Statement::ASSIGN, "x", Expr(10)),
    Statement(Statement::DEF, "f", std::vector<Statement>{
        Statement(Statement::PRINT, Expr("x"))
    }),
    Statement(Statement::DEF, "g", std::vector<Statement>{
        Statement(Statement::ASSIGN, "x", Expr(20)),
        Statement(Statement::CALL, "f")
    })
};

int main() {
    // Main function to demonstrate both scoping modes
    
    std::cout << "Static Scoping Output:" << std::endl;
    // Run with static scoping
    // In static scoping, when f() is called from g(), it looks up x in f's definition environment
    // f was defined in global scope where x=10, so it prints 10
    Environment* env_static = run_program(example_program, "static");
    // Now call g() which will call f()
    Statement call_g(Statement::CALL, "g");
    execute_stmt(call_g, env_static, "static");
    
    std::cout << "\nDynamic Scoping Output:" << std::endl;
    // Run with dynamic scoping
    // In dynamic scoping, when f() is called from g(), it looks up x in g's environment
    // g has x=20, so f() prints 20
    Environment* env_dynamic = run_program(example_program, "dynamic");
    // Now call g() which will call f()
    execute_stmt(call_g, env_dynamic, "dynamic");
    
    // Clean up
    delete env_static;
    delete env_dynamic;
    
    return 0;
}
