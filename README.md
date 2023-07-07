# PyBridge

PyBridge: Integration for Windows applications (Electron, etc.), enabling asynchronous calls to Python 3.8.10 functions from Node.js, using Pybind11 and an independent C++ thread pool.

## Table of Contents
1. [Getting Started](#getting-started)
2. [Usage](#usage)
3. [Contributing](#contributing)
4. [Changelog](#changelog)
5. [Tips](#tips)
6. [License](#license)

## Getting Started

1. PyBridge provides Python call support through Node extension bindings, utilizing an independent thread pool instead of Libuv in Node.js.
2. This has two major advantages: first, it keeps the two systems isolated; second, it circumvents the limitations of the Python GIL lock. Submitting multiple tasks, especially compute-intensive ones, could otherwise lead to Libuv thread pool blocking. 
3. PyBridge is designed with Python 3.8.10 in mind, which is the last version supporting Windows 7. The repository includes headers, libraries, and DLLs required by node-gyp. PyBridge supports both TypeScript and ESM/CJS modules in your application.

### Prerequisites

- Node.js (version 16 or higher) (Lower versions of node.js have not been tested)
- Python 3.8.10
- Pybind11 (version 2.10.4)

### Installation

1. Install Node.js, Python, and the C++ Compiler.

    Installing the C++ compiler is needed.

    ```
    npm install --global --production windows-build-tools
    ```

    If you have already installed **python**, **vs2019** or higher versions, you don't need to install **windows-build-tools**.

    The project is compiled using node-gyp, thus requiring the Node.js addon compiler.

2. Install PyBridge from npm.

    ``` shell
    npm install py-bridge
    ```

## Usage

In the Node.js environment, you can use JS/TS according to your preference.
Both CommonJS and ESM packages will be installed by Rollup.

### Basic Usage

Calling a simple python function.

Python script:

app.py

```python
    import time

    def call(a, b, c, d):
        time.sleep(2)
        print(a, b, c, d)
        return [{"test": "test_result"}]

    def error(s):
        raise Exception(s)
```

Typescript:

First:
1. Initialize the python interpreter.
* pythonHome is the path to the python root directory.
* pythonPath is the path to store your python script.
* The third parameter represents the number of threads in the thread pool.If you set this to null or undefined, the number of threads will be setted to **std::thread::hardware_concurrency()**

```typescript
import { interpreter, ArgumentType } from 'py-bridge'

interpreter.initialize(pythonHome, pythonPath, undefined);
```

Second: 
* Usage of decorator functions and normal addon api to call python function.
* Tips:  The Args (referring to a certain type) and {test: string}[] (an array of objects with a 'test' property of type string) must both be a subset of the 'ArgumentType' type. 

```typescript

interface Args {
    a: number;
    b: {
        name: string;
        age: number;
    }[];
    c: number[];
    d: { has_child: boolean };
}

class Test {
    // A Python function caller is created using a classmethod decorator.
    // This uses the TypeScript 5 classmethod decorator. If you require TypeScript 4, especially for metadata, this method won't be suitable.
    @interpreter.definePyFunction("app", "call")
    call(_: Args): Promise<{test: string}[]> {
        return new Promise(() => {});
    }
}

const data = {
    a: 1,
    b: [
        {name: "name1", age: 2},
        {name: "name2", age: 3}
    ],
    c: [1, 2, 3, 4],
    d: {has_child: true}
}

const test = new Test();
test.call(data).then((res) => {
    console.log("The Python function result returned by the decorated function: ", res)
}).catch((err) => {
    console.log(err)
});

// The Python function is called using the normal add-on API.
interpreter.callAsync<typeof data, number>("app", "call", data).then((res) => {
    console.log("The Python function result via the addon API: ", res)
}).catch((err) => {
    console.log(err)
});

```

Third:
* Release the interpreter.

```typescript
interpreter.finalize();
```

## Contributing

As a new coder myself, I warmly welcome code contributions to PyBridge. Please don't hesitate to submit your suggestions or report bugs.

## Changelog

1. 2023/07/07: First Version.

## Tips

### Generate Your Addon Projects

1. Currently, this package only supports Python 3.8.10 on Windows due to its compatibility with Windows 7, which is the latest version supported by Python 3.8.10.

2. If you require other add-on versions, feel free to fork this project and generate your customized add-on project.

### Future 

1. Add support for dynamic thread pool.

2. Expand data mapping support.

## License

PyBridge is licensed under [INSERT LICENSE HERE]. See the LICENSE file for details.
