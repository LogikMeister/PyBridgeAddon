export declare function makeGuardDecorator<T, A extends any[]>(fn: (t: T, instance: any, ...args: A) => boolean, errMessage: string): (t: T) => <This, Args extends A, Return>(target: (this: This, ...args: Args) => Return, context: ClassMethodDecoratorContext<This, (this: This, ...args: Args) => Return>) => (this: This, ...args: Args) => Return;
