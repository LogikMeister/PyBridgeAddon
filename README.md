![PyBridgeAddon](https://github.com/LogikMeister/PyBridgeAddon/blob/master/logo.png)

# PyBridgeAddon

PyBridgeAddon: Integration for Windows applications (Electron, etc.), enabling asynchronous calls to Python functions from Node.js, using Pybind11 and an independent C++ thread pool.

**[New Change](#changelog): 2023/07/10**

## Table of Contents
1. [Getting Started](#getting-started)
2. [Usage](#usage)
3. [Contributing](#contributing)
4. [Changelog](#changelog)
5. [Tips](#tips)
6. [License](#license)

## Getting Started

1. PyBridgeAddon provides Python call support through Node extension bindings, utilizing `an independent thread pool instead of Libuv` in Node.js.
2. This has two major advantages: first, it keeps the two systems isolated; second, it circumvents the limitations of the Python GIL lock. Submitting multiple tasks, especially compute-intensive ones, could otherwise lead to Libuv thread pool blocking. 
3. PyBridgeAddon is designed to work with Python 3.X. **The package automatically searches for the necessary Python headers and libraries using node-gyp in PATH environment variable**.
4. When you install `'pybridge-addon'` using npm, the `python.dll` and `pythonXX.dll` files required by the Python interpreter will be **automatically copied to the "dll" folder in the root directory of the package**. This ensures that the necessary dll files are included in the system path (by adding the "dll" folder to the PATH environment variable), guaranteeing the distribution of your Electron application or other applications.
4. PyBridgeAddon supports both `TypeScript` and `ESM/CJS` modules in your application.

<div align=center><img src="https://github.com/LogikMeister/PyBridgeAddon/blob/master/prototype.png" alt="Prototype" width="400"></div>


### Prerequisites

- `Node.js` (version 16 or higher) (Lower versions of node.js have not been tested)
- `Python` 3.X (Make sure to add the Python directory to your path environment variable to ensure successful installation of the package.)

### Installation

1. Install Node.js, Python, and the C++ Compiler.

    Installing the C++ compiler is needed.

    ```
    npm install --global --production windows-build-tools
    ```

    If you have already installed `python`, `vs2019` or higher versions, you don't need to install `windows-build-tools`.

    The project is compiled using `node-gyp`, thus requiring the Node.js addon compiler.

2. Install PyBridgeAddon from npm.

    ``` shell
    npm install pybridge-addon
    ```

## Usage

In the Node.js environment, you can use `JS/TS` according to your preference.
Both `CommonJS` and `ESM` packages will be installed by Rollup.

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
* `pythonHome` is the path to the python root directory.

* In the Python root directory, the `DLLs` and `Lib` are the only directories that need to be retained. All others can be deleted.

  `DLLs`: This directory contains the dynamic link libraries (DLLs) required for Python runtime. DLLs provide core and standard functionality needed when Python is running.

  `Lib`: This directory houses Python's standard library, a comprehensive suite of modules for various development tasks. The presence of this library makes Python a "batteries included" language, capable of supporting many common programming tasks right out of the box.

* `pythonPath` is the path to store your python script.

* `The third parameter` represents the number of threads in the thread pool. If you set this to null or undefined, the number of threads will be setted to `std::thread::hardware_concurrency()`

* ```
  PythonXX/
  ├── DLLs/ ========================> Need To Keep
  ├── Doc/  --  --  --  --  --  --  - Can Delete
  ├── include/ --  --  --  --  --  -- Can Delete
  ├── Lib/ =========================> Need To Keep
  │   └── site-packages/
  ├── libs/ -  --  --  --  --  --  -- Can Delete
  ├── Scripts/ --  --  --  --  --  -- Can Delete
  │   ├── pip.exe
  │   ├── pipX.exe
  │   ├── pipX.X.exe
  │   └── ...
  ├── Tools/ - --  --  --  --  --  -- Can Delete
  ├── python.exe   --  --  --  --  -- Can Delete
  ├── pythonw.exe  --  --  --  --  -- Can Delete
  └── pythonXX.dll --  --  --  --  -- Can Delete. When you install 'pybridge-addon' using npm and execute your code, the directories containing the required pythonXX.dll will be automatically added to the PATH environment variable.
  
  ```

```typescript
import { interpreter, ArgumentType } from 'pybridge-addon'

interpreter.initialize(pythonHome, pythonPath, undefined);
```

Second: 
* Usage of decorator functions and normal addon api to call python function.
* Tips:  The `'Args'` (referring to a certain type) and `'Return'` must both be a subset of the `'ArgumentType'` type. 

```typescript

// The use of interface Args {} is not supported because interfaces enforce a stricter structure.
// you can use type to define the data structure, 
// which provides more flexibility for varying property keys, 
// including those not known at the time of definition.

type Args = {
    a: number;
    b: {
        name: string;
        age: number;
    }[];
    c: number[];
    d: { has_child: boolean };
}

type Return {
   test: string; 
}[]

class Test {
    // A Python function caller is created using a classmethod decorator.
    // This uses the TypeScript 5 classmethod decorator. 
    // If you require TypeScript 4, especially for metadata, this method won't be suitable.
    @interpreter.definePyFunction("app", "call")
    call(_: Args): Promise<Return> {
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
interpreter.callAsync<typeof data, Return>("app", "call", data).then((res) => {
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


### Event Emitter

The `event_trigger` module, dynamically generated by pybind11, is readily available for import.
You can utilize `the event_trigger.emit()` method directly in your python script.

```python
import event_trigger

def call():
    event_trigger.emit("data", 123)
    return [{"test": "test_result"}]
```

```typescript
import { interpreter, event, ArgumentType } from '../dist/esm/index.js'

interpreter.initialize(pythonHome, pythonPath, undefined);

type Return = {test: string}[];

class Test {
    num = 1;

    @interpreter.definePyFunction("app", "call")
    call(): Promise<Return> {
        return new Promise(() => {});
    }

    // This uses the TypeScript 5 classmethod decorator. 
    // If you require TypeScript 4, especially for metadata, this method won't be suitable.
    @event.definePyEvent("app", "call", "data")
    onData(data: number) {
        console.log("The Python function \"data\" event: ", data + this.num);
    }
}

// You can definePyEvent using the normal add-on API.
//
// event.set("app", "call", "data", (data: number) => {});
//

const test = new Test();

test.call().then((res) => {
    console.log("The Python function result returned by the decorated function: ", res)
}).catch((err) => {
    console.log(err)
});

```

## Contributing

As a new coder myself, I warmly welcome code contributions to PyBridgeAddon. Please don't hesitate to submit your suggestions or report bugs.

## Changelog

1. 2023/07/07: First Version.
2. 2023/07/10: Fixed bugs and added tests. The package now dynamically searches for the appropriate version of Python in your PATH environment variable, ensuring compatibility across different Python versions.

## Tips

### Future 

1. Add support for dynamic thread pool.

2. Expand data mapping support.

3. Add support for mac or linux.

## License

PyBridgeAddon is licensed under [MIT]. See the [LICENSE](LICENSE) file for details.
