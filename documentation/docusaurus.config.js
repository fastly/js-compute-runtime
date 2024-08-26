// @ts-check
// Note: type annotations allow type checking and IDEs autocompletion

import { themes } from 'prism-react-renderer';

const lightCodeTheme = themes.github;
const darkCodeTheme = themes.dracula;

/** @type {import('@docusaurus/types').Config} */
export default {
  webpack: {
    jsLoader: (isServer) => ({
      loader: require.resolve('swc-loader'),
      options: {
        jsc: {
          parser: {
            syntax: 'ecmascript',
            jsx: true,
          },
          target: 'es2017',
          transform: {
            react: {
              runtime: 'automatic'
            }
          }
        },
        module: {
          type: isServer ? 'commonjs' : 'es6',
        },
      },
    }),
  },

  plugins: [
    require.resolve("@cmfcmf/docusaurus-search-local"),
  ],
  title: '@fastly/js-compute',
  tagline: 'Javascript SDK and CLI for Fastly Compute',
  url: 'https://js-compute-reference-docs.edgecompute.app',
  baseUrl: '/',
  onBrokenLinks: 'throw',
  onBrokenMarkdownLinks: 'throw',
  favicon: './fastly-favicon-default.svg',

  organizationName: 'fastly',
  projectName: 'js-compute-runtime',
  i18n: {
    defaultLocale: 'en',
    locales: ['en'],
  },

  presets: [
    [
      'classic',
      /** @type {import('@docusaurus/preset-classic').Options} */
      ({
        docs: {
          includeCurrentVersion: false,
        },
        theme: {
          customCss: require.resolve('./src/css/custom.css'),
        },

      }),
    ],
  ],

  staticDirectories: ['static'],

  scripts: [
    '/fiddle.js',
  ],

  themeConfig:
    /** @type {import('@docusaurus/preset-classic').ThemeConfig} */
    ({
      navbar: {
        title: '@fastly/js-compute',
        items: [
          {
            type: 'docsVersionDropdown',
          },
          {href: 'https://developer.fastly.com/', label: 'Fastly Developer Hub', position: 'left'},
          {href: 'https://developer.fastly.com/solutions/examples/javascript/', label: 'Code Examples', position: 'left'},
          {href: 'https://fiddle.fastly.dev', label: 'Fastly Fiddle', position: 'left'},
          {
            href: 'https://twitter.com/FastlyDevs/',
            label: 'Twitter',
            position: 'right',
          },
          {
            href: 'https://github.com/fastly/js-compute-runtime/',
            label: 'GitHub',
            position: 'right',
          },
        ],
      },
      footer: {
        style: 'dark',
        copyright: `Copyright © ${new Date().getFullYear()} Fastly Inc. All rights reserved. Portions of this content are ©1998–2023 by individual mozilla.org contributors. Content available under a <a href="https://creativecommons.org/licenses/by-sa/2.5/">CC-BY-SA 2.5</a> license.`,
      },
      prism: {
        theme: lightCodeTheme,
        darkTheme: darkCodeTheme,
      },
    }),
};
