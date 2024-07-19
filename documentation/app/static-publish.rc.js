/*
 * Copyright Fastly, Inc.
 * Licensed under the MIT license. See LICENSE file for details.
 */

// Commented items are defaults, feel free to modify and experiment!
// See README for a detailed explanation of the configuration options.

/** @type {import('@fastly/compute-js-static-publish').StaticPublisherConfig} */
const config = {
  rootDir: "../build",
  staticContentRootDir: "./static-publisher",
  kvStoreName: "js-docs-site",
  contentCompression: [], // For this config value, default is [] if kvStoreName is null.
  server: {
    publicDirPrefix: "",
    staticItems: ["/"],
    compression: [],
    spaFile: false,
    notFoundPageFile: "/404.html", 
    autoExt: [],
    autoIndex: ["index.html","index.htm"],
  },
};

export default config;
