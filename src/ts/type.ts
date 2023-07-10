type BasicType = string | number | boolean | null | undefined

interface ArgumentObject {
    [key: string]: BasicType | Array<BasicType> | ArgumentObject | Array<ArgumentObject>
}

export type ArgumentType = BasicType | ArgumentObject | Array<ArgumentObject> | Array<BasicType>

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
