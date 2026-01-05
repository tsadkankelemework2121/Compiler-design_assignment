"""
Scoping Demo - Static vs Dynamic Scoping
This program demonstrates the difference between static (lexical) and dynamic scoping.
"""

class Environment:
    """Represents a scope/environment where variables are stored."""
    def __init__(self, parent=None):
        # I'm using a dictionary to store variable bindings
        self.vars = {}
        # Parent environment for nested scopes
        self.parent = parent
    
    def get(self, name):
        """Look up a variable in this environment or parent."""
        if name in self.vars:
            return self.vars[name]
        if self.parent:
            return self.parent.get(name)
        raise NameError(f"Variable '{name}' not found")
    
    def set(self, name, value):
        """Set a variable in this environment."""
        self.vars[name] = value


class Function:
    """Represents a function definition."""
    def __init__(self, name, body, definition_env=None):
        self.name = name
        self.body = body  # List of statements to execute
        # For static scoping: capture the environment where function is defined
        self.definition_env = definition_env
    
    def call(self, call_env, scoping_mode='static'):
        """
        Execute the function body.
        scoping_mode: 'static' uses definition_env, 'dynamic' uses call_env
        """
        if scoping_mode == 'static':
            # Static scoping: use the environment where function was defined
            # This is the key difference - we use definition_env, not call_env
            env = Environment(parent=self.definition_env)
        else:
            # Dynamic scoping: use the environment where function is called
            # This means we look up variables in the call stack
            env = Environment(parent=call_env)
        
        # Execute function body in the chosen environment
        result = None
        for stmt in self.body:
            result = execute_stmt(stmt, env, scoping_mode)
        return result


def execute_stmt(stmt, env, scoping_mode='static'):
    """Execute a single statement."""
    stmt_type = stmt[0]
    
    if stmt_type == 'assign':
        # Assignment: x = value
        var_name = stmt[1]
        value = execute_expr(stmt[2], env, scoping_mode)
        env.set(var_name, value)
        return value
    
    elif stmt_type == 'print':
        # Print statement: print(expr)
        value = execute_expr(stmt[1], env, scoping_mode)
        print(value)
        return value
    
    elif stmt_type == 'call':
        # Function call: func_name()
        func_name = stmt[1]
        func = env.get(func_name)
        if not isinstance(func, Function):
            raise TypeError(f"'{func_name}' is not a function")
        # When calling, pass current environment for dynamic scoping
        return func.call(env, scoping_mode)
    
    elif stmt_type == 'def':
        # Function definition: def func_name(): body
        func_name = stmt[1]
        body = stmt[2]
        # Create function and capture current environment for static scoping
        func = Function(func_name, body, definition_env=env)
        env.set(func_name, func)
        return func


def execute_expr(expr, env, scoping_mode='static'):
    """Evaluate an expression."""
    if isinstance(expr, int):
        # Integer literal
        return expr
    elif isinstance(expr, str):
        # Variable reference
        return env.get(expr)
    else:
        raise ValueError(f"Unknown expression type: {expr}")


def run_program(program, scoping_mode='static'):
    """
    Run a program with the specified scoping mode.
    program: list of statements
    scoping_mode: 'static' or 'dynamic'
    """
    # Global environment
    global_env = Environment()
    
    # Execute all statements
    for stmt in program:
        execute_stmt(stmt, global_env, scoping_mode)
    
    return global_env


# Define the example program
# x = 10
# def f():
#     print(x)
# def g():
#     x = 20
#     f()

example_program = [
    ('assign', 'x', 10),
    ('def', 'f', [
        ('print', 'x')
    ]),
    ('def', 'g', [
        ('assign', 'x', 20),
        ('call', 'f')
    ])
]


def main():
    """Main function to demonstrate both scoping modes."""
    
    print("Static Scoping Output:")
    # Run with static scoping
    # In static scoping, when f() is called from g(), it looks up x in f's definition environment
    # f was defined in global scope where x=10, so it prints 10
    env_static = run_program(example_program, scoping_mode='static')
    # Now call g() which will call f()
    execute_stmt(('call', 'g'), env_static, scoping_mode='static')
    
    print("\nDynamic Scoping Output:")
    # Run with dynamic scoping
    # In dynamic scoping, when f() is called from g(), it looks up x in g's environment
    # g has x=20, so f() prints 20
    env_dynamic = run_program(example_program, scoping_mode='dynamic')
    # Now call g() which will call f()
    execute_stmt(('call', 'g'), env_dynamic, scoping_mode='dynamic')


if __name__ == '__main__':
    main()

