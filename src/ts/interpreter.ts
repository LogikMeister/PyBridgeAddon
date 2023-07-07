import { ArgumentType, Addon } from "./type";
import { makeGuardDecorator } from './decorator';
import { createRequire } from 'node:module';
import { fileURLToPath } from 'node:url';
import path from 'node:path';
import process from 'node:process';

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

const require = createRequire(import.meta.url);
const resolve = (p: string) => path.resolve(__dirname, p);

let addon: Addon;
try {
    const dynamic_linked_library_path = resolve('../../dll');
    process.env.PATH = dynamic_linked_library_path + ";" + process.env.PATH;
    addon = require(resolve('../../build/Release/pybridge'));
} catch (e) {
    throw new Error('[Error]: Failed to load pybridge addon. Please check if pythonXX.dll exists in the environment variable PATH.');
}

const isFalse = makeGuardDecorator(
    (property: string, instance: any,..._rest: any[]) => instance[property] === false, 
    "Python interpreter has not been initialized"
);

class Interpreter {
    private isInitialized = false;
    private pythonHome: string | undefined;
    private pythonPath: string | undefined;

    @isFalse("isInitialized")
    public initialize(pythonHome: string, pythonPath: string, threads?: number): boolean {
        this.pythonHome = pythonHome;
        this.pythonPath = pythonPath;
        try {
            addon.initializePython(this.pythonHome, this.pythonPath, threads);
            const res = addon.isInitialized();
            if (res) {
                this.isInitialized = true;
                return this.isInitialized;
            } else {
                throw new Error();
            }
        } catch (e) {
            this.isInitialized = false;
            console.error("[Error]:", (e as Error).message);
            throw new Error(`Failed to initialize Python interpreter at ${pythonHome}.`);
        }
    }

    public finalize() {
        if (this.isInitialized && addon.isInitialized()) {
            addon.finalizePython();
        }
        this.isInitialized = false;
    }

    public call<A extends ArgumentType, R extends ArgumentType>(moduleName: string, methodName: string, args: A): R {
        if (!this.isInitialized) {
            throw new Error("Python interpreter has not been initialized.");
        }
        return addon.callPythonFunctionSync<A, R>(moduleName, methodName, args);
    }

    public callAsync<A extends ArgumentType, R extends ArgumentType>(moduleName: string, methodName: string, args: A): Promise<R> {
        if (!this.isInitialized) {
            throw new Error("Python interpreter has not been initialized.");
        }
        return addon.callPythonFunctionAsync(moduleName, methodName, args);
    }

    public definePyFunction<A extends ArgumentType, R extends ArgumentType>(moduleName: string, methodName: string) {
        // Method Decorator
        const instance = this;
        return function <This, Args extends A, Return extends R>(
            target: (this: This, args: Args) => Promise<Return>,
            content: ClassMethodDecoratorContext<
                This,
                (this: This, args: Args) => Promise<Return>
            >
        ) {
            const replaceMethod = function (this: This, args: Args ) {
                return instance.callAsync<Args, Return>(moduleName, methodName, args);
            }
            return replaceMethod;
        }
    }
}

const interpreter = new Interpreter();

export default interpreter;