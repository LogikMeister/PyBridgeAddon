import { ArgumentType } from './type';
declare class Interpreter {
    private isInitialized;
    private pythonInitalizFailed;
    private pythonHome;
    private pythonPath;
    initialize(pythonHome: string, pythonPath: string, threads?: number): boolean;
    finalize(): boolean;
    call<A extends ArgumentType, R extends ArgumentType>(moduleName: string, methodName: string, args?: A): R;
    callAsync<A extends ArgumentType, R extends ArgumentType>(moduleName: string, methodName: string, args?: A): Promise<R>;
    definePyFunction(moduleName: string, methodName: string): <This, Args extends ArgumentType, Return extends ArgumentType>(target: (this: This, args: Args) => Promise<Return>, context: ClassMethodDecoratorContext<This, (this: This, args: Args) => Promise<Return>>) => (this: This, args?: Args | undefined) => Promise<Return>;
}
declare const interpreter: Interpreter;
export default interpreter;
