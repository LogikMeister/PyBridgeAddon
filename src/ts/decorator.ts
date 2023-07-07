export function makeGuardDecorator<T, A extends any[]>(
    fn: (t: T, instance: any, ...args: A) => boolean,
    errMessage: string
) {
    return function (t: T) {
        // Method Decorator
        return function <This, Args extends A, Return>(
            target: (this: This, ...args: Args) => Return,
            context: ClassMethodDecoratorContext<
                This,
                (this: This, ...args: Args) => Return
            >
        ) {
            const methodName = String(context.name);

            const replaceMethod = function (this: This, ...args: Args) {
                if (fn(t, this, ...args)) {
                    return target.call(this, ...args);
                }
                // 如果校验失败
                throw new Error(`${methodName}: ${errMessage}, arg: ${t}`);
            };

            return replaceMethod;
        };
    };
}