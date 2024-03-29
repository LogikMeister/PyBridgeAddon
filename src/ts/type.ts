type BasicType = string | number | boolean | null | undefined

interface ArgumentObject {
    [key: string]: BasicType | ArgumentArray | ArgumentObject | Array<ArgumentObject>
}

type ArgumentArray = Array<BasicType> | Array<ArgumentArray> | Array<ArgumentObject>

export type ArgumentType = BasicType | ArgumentObject | ArgumentArray

export interface Addon {
    isInitialized(): boolean
    initializePython(pythonHome: string, pythonPath: string, threads?: number): boolean
    finalizePython(): boolean
    callPythonFunctionSync<A extends ArgumentType, R extends ArgumentType>(
        moduleName: string,
        functionName: string,
        functionArgs?: A
    ): R
    callPythonFunctionAsync<A extends ArgumentType, R extends ArgumentType>(
        moduleName: string,
        functionName: string,
        functionArgs?: A,
        handlers?: EventMap
    ): Promise<R>
}

export type EventHandler<A extends ArgumentType> = (args: A) => void
export type EventMap = { [event: string]: EventHandler<any> }
export type EventGroupType = { moduleName: string; methodName: string }
