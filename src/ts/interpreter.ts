import event from './event'
import { ArgumentType, Addon } from './type'
import { makeGuardDecorator } from './decorator'
import path from 'node:path'
import process from 'node:process'

const resolve = (p: string) => path.resolve(__dirname, p)

let addon: Addon
try {
    const dynamic_linked_library_path = resolve('../../dll')
    process.env.PATH = dynamic_linked_library_path + ';' + process.env.PATH
    addon = require(resolve('../../build/Release/pybridge'))
} catch (e) {
    throw new Error(
        '[Addon Import Error]: Failed to load pybridge addon. Please check if pythonXX.dll exists in the environment variable PATH.'
    )
}

const needUnintialized = makeGuardDecorator(
    (property: string, instance: any, ..._rest: any[]) => instance[property] === false,
    'Python interpreter has not been initialized'
)

const needUnintializFailed = makeGuardDecorator(
    (property: string, instance: any, ..._rest: any[]) => instance[property] === false,
    'Python interpreter initialization failure is destructive. You need to restart you node application to initialize Python interpreter.'
)

class Interpreter {
    private isInitialized = false
    private pythonInitalizFailed = false
    private pythonHome: string | undefined
    private pythonPath: string | undefined

    @needUnintializFailed('pythonInitalizFailed')
    @needUnintialized('isInitialized')
    public initialize(pythonHome: string, pythonPath: string, threads?: number): boolean {
        this.pythonHome = pythonHome
        this.pythonPath = pythonPath
        try {
            addon.initializePython(this.pythonHome, this.pythonPath, threads)
            addon.isInitialized()
            this.isInitialized = true
            return this.isInitialized
        } catch (e) {
            this.isInitialized = false
            this.pythonInitalizFailed = true
            throw new Error(`Failed to initialize Python interpreter at ${pythonHome}.`)
        }
    }

    public finalize() {
        let isSuccess = false
        if (this.isInitialized && addon.isInitialized()) {
            isSuccess = addon.finalizePython()
        }
        this.isInitialized = false
        return isSuccess
    }

    public call<A extends ArgumentType, R extends ArgumentType>(
        moduleName: string,
        methodName: string,
        args?: A
    ): R {
        if (!this.isInitialized) {
            throw new Error('Python interpreter has not been initialized.')
        }
        return addon.callPythonFunctionSync<A, R>(moduleName, methodName, args)
    }

    public callAsync<A extends ArgumentType, R extends ArgumentType>(
        moduleName: string,
        methodName: string,
        args?: A
    ): Promise<R> {
        if (!this.isInitialized) {
            throw new Error('Python interpreter has not been initialized.')
        }
        const handlers = event.getHandlers(moduleName, methodName)
        return addon.callPythonFunctionAsync(moduleName, methodName, args, handlers)
    }

    public definePyFunction(moduleName: string, methodName: string) {
        // Method Decorator
        const instance = this
        return function <This, Args extends ArgumentType, Return extends ArgumentType>(
            target: (this: This, args: Args) => Promise<Return>,
            context: ClassMethodDecoratorContext<This, (this: This, args: Args) => Promise<Return>>
        ) {
            const replaceMethod = function (this: This, args?: Args) {
                return instance.callAsync<Args, Return>(moduleName, methodName, args)
            }
            return replaceMethod
        }
    }
}

const interpreter = new Interpreter()

export default interpreter
