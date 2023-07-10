import terser from '@rollup/plugin-terser'
import typescript from '@rollup/plugin-typescript'
import resolve from '@rollup/plugin-node-resolve'
import commonjs from '@rollup/plugin-commonjs'

export default {
    input: 'src/ts/index.ts',
    output: [
        {
            file: 'dist/cjs/index.js',
            format: 'cjs',
            sourcemap: true,
            plugins: [
                // terser()
            ]
        },
        {
            file: 'dist/esm/index.js',
            format: 'esm',
            sourcemap: true,
            plugins: [
                // terser()
            ]
        }
    ],
    plugins: [
        typescript({
            tsconfig: './tsconfig.rollup.json',
            sourceMap: true
        }),
        resolve(),
        commonjs()
    ]
}
