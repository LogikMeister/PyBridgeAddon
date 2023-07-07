import terser from "@rollup/plugin-terser"
import typescript from "@rollup/plugin-typescript"
import resolve from "@rollup/plugin-node-resolve"
import commonjs from '@rollup/plugin-commonjs'

export default {
    input: "src/ts/index.ts",
    output: [
        {
            file: "dist/cjs/index.js",
            format: "cjs",
            plugins: [
                terser()
            ]
        },
        {
            file: "dist/esm/index.js",
            format: "esm",
            plugins: [
                terser()
            ]
        }
    ],
    plugins: [
        typescript({
            tsconfig: "./tsconfig.json"
        }),
        resolve(),
        commonjs()
    ]
}