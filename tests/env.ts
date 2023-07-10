import path from 'path'

const pythonHome = process.env.PYTHON_HOME || path.join(__dirname, '..', 'resource', 'python')
const pythonPath = path.join(__dirname, 'script')

export { pythonHome, pythonPath }
