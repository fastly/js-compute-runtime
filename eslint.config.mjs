// @ts-check

import eslint from '@eslint/js';
import { defineConfig } from 'eslint/config';
import tseslint from 'typescript-eslint';

export default defineConfig([
  {
    files: ['src/**/*.{ts,tsx,js}'],
    languageOptions: {
      parserOptions: {
        projectService: true,
      },
    },
    extends: [
      eslint.configs.recommended,
      ...tseslint.configs.recommendedTypeChecked,
      ...tseslint.configs.stylisticTypeChecked,
    ],
    rules: {
      'no-fallthrough': 'off',
      '@typescript-eslint/require-await': 'off',
      '@typescript-eslint/consistent-type-definitions': 'off',
    },
  },
  {
    ignores: [
      'ci/*',
      'dist/*',
      'documentation/*',
      'integration-tests/*',
      'runtime/*',
      'tests/*',
      'test-d/*',
    ]
  },
]);
