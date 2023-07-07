import { ArgumentType } from "./type";
declare class Interpreter {
    private isInitialized;
    private pythonHome;
    private pythonPath;
    initialize(pythonHome: string, pythonPath: string, threads?: number): boolean;
    finalize(): void;
    call<A extends ArgumentType, R extends ArgumentType>(moduleName: string, methodName: string, args: A): R;
    callAsync<A extends ArgumentType, R extends ArgumentType>(moduleName: string, methodName: string, args: A): Promise<R>;
    definePyFunction<A extends ArgumentType, R extends ArgumentType>(moduleName: string, methodName: string): <This, Args extends A, Return extends R>(target: (this: This, args: Args) => Promise<Return>, content: ClassMethodDecoratorContext<This, (this: This, args: Args) => Promise<Return>>) => (this: This, args: Args) => Promise<Return>;
}
declare const interpreter: Interpreter;
export default interpreter;
