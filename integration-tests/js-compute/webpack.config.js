const path = require("path");

module.exports = {
  stats: { errorDetails: true },
  target: "es2019",
  /*
  output: {
    path: path.join(process.cwd(), "build"),
    filename: "bundle.js",
  },
  */
  // mode: 'production',
  mode: "development",
  devtool: "cheap-module-source-map",
  optimization: {
    sideEffects: true,
  },
  resolve: {
    extensions: [".tsx", ".ts", ".js", ".json"],
  },
  module: {
    rules: [
      // all files with a '.ts' or '.tsx' extension will be handled by 'ts-loader'
      { test: /\.tsx?$/, use: ["ts-loader"], exclude: /node_modules/ },
    ],
  },
};
