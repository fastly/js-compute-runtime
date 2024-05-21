#!/usr/bin/env node

import { readFileSync, writeFileSync } from "node:fs";
import path from "node:path";
import process from "node:process";
import { unified } from "unified";
import remarkParse from "remark-parse";
import remarkStringify from "remark-stringify";

let markdownString;
const fileFromArgs = process.argv[2];
const filePath = path.join(process.cwd(), fileFromArgs);
try {
  markdownString = readFileSync(filePath, "utf-8");
} catch (err) {
  console.log(
    `Could not read or maybe even find your markdown file. \nCheck your file path, name, extension. \nMake sure you type 'npm run check -- pathtofile.md'\nError from Node.js: ${err}`
  );
  process.exit(1);
}

let ast;
try {
  ast = getAst(markdownString);
} catch (err) {
  console.log(`Could not parse the markdown into a syntax tree: ${err}`);
  process.exit(1);
}

try {
  const result = format(ast, filePath);

  if (result.changed) {
    console.log("Updated markdown");
  }
  if (result.correct) {
    console.log("Looks like the markdown was good enough");
    process.exit(0);
  } else {
    console.log(`There was a problem with the markdown...\n ${result.reason}`);
    process.exit(1);
  }
} catch (err) {
  console.log(
    `I must have made a mistake or not handled an error, soz\n${err}`
  );
  process.exit(1);
}

function getAst(markdownString) {
  const tree = unified().use(remarkParse).parse(markdownString);
  return tree;
}

function format(ast, path) {
  try {
    let changed = false;
    const content = ast.children;
    if (!content.length) {
      return {
        correct: false,
        reason: "Empty file maybeee",
      };
    }

    // heading 1 is optional so if it's there just get rid of it and check the rest
    if (content[0].type === "heading" && content[0].depth === 1) {
      content.splice(0, 1);
    }

    // now we may have removed the h1, the new 'first' item should be a h2
    // check first item is a heading
    if (content[0].type !== "heading" || content[0].depth !== 2) {
      return {
        correct: false,
        reason:
          "There should be a level 2 heading at the top, or immediately after the level 1 heading if you have one. If you have a level 1 heading, make sure there is no text between that and the level 2 heading",
      };
    }

    for (let i = 0; i < content.length; i++) {
      // checks on all ## headings
      const item = content[i];
      if (item.type === "heading" && item.depth === 2) {
        // check correct amount of text is at heading 2
        if (
          item.children.length === 2 &&
          item.children[0].type === "link" &&
          item.children[0].children.length === 1 &&
          item.children[0].children[0].type === "text" &&
          item.children[1].type === "text"
        ) {
          const link = item.children[0];
          item.children = [item.children[1]];
          item.children[0].value =
            link.children[0].value + item.children[0].value;
          changed = true;
        } else if (item.children.length !== 1) {
          console.log(
            `${path}:${item.position.start.line}:${item.position.start.column}`
          );
          return {
            correct: false,
            reason:
              "Level 2 headings should say version and date, e.g. 1.9.2 (2023-02-10) and contain no other markdown",
          };
        }

        const heading2 = item.children[0];
        const heading2Text = heading2.value;

        // check heading 2 text can be split into exactly 2 parts at the point of a space
        let textParts;
        try {
          textParts = heading2Text.split(" ");
        } catch (err) {
          console.log(
            `${path}:${heading2.position.start.line}:${heading2.position.start.column}`
          );
          return {
            correct: false,
            reason: `Level 2 headings should contain a space. We expect one between the semantic version number and the date. Error message: ${err}`,
          };
        }

        if (textParts.length > 2) {
          console.log(
            `${path}:${heading2.position.start.line}:${heading2.position.start.column}`
          );
          return {
            correct: false,
            reason:
              "Level 2 headings should only contain one space. We expect one between the semantic version number and the date",
          };
        }

        // check first part of header 2 is a semantic version number
        const expectedSemanticVersion = textParts[0];
        if (
          !/^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)(?:-((?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\+([0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$/.test(
            expectedSemanticVersion
          )
        ) {
          console.log(
            `${path}:${heading2.position.start.line}:${heading2.position.start.column}`
          );
          return {
            correct: false,
            reason:
              "First part of level 2 headings should be a semantic version, e.g. 1.9.2",
          };
        }

        // check second part of header 2 is a date in format YYYY-MM-DD
        const expectedDate = textParts[1];
        if (
          !/^\((\d{4,5}-(0[1-9]|1[0-2])-(0[1-9]|[12][0-9]|3[01])\))$/.test(
            expectedDate
          )
        ) {
          console.log(
            `${path}:${heading2.position.start.line}:${heading2.position.start.column}`
          );
          return {
            correct: false,
            reason:
              "Second part of level 2 headings should be a hypen-separated date in the format YYYY-MM-DD",
          };
        }

        // check it is followed by at least one level 3 heading
        if (
          !content[i + 1] ||
          content[i + 1].type !== "heading" ||
          content[i + 1].depth !== 3
        ) {
          console.log(
            `${path}:${item.position.start.line}:${item.position.start.column}`
          );
          return {
            correct: false,
            reason:
              "Level 2 headings must be followed by at least one level 3 heading",
          };
        }
      }

      // checks on all ### headings
      if (item.type === "heading" && item.depth === 3) {
        // check it only uses one of the fixed options for change types
        if (item.children.length === 1 && item.children[0].type === "text") {
          const val = item.children[0].value;
          if (val.toLowerCase().endsWith("breaking changes")) {
            item.children[0].value = "Changed";
            changed = true;
          } else if (val.includes("Bug Fixes")) {
            item.children[0].value = "Fixed";
            changed = true;
          } else if (val.includes("Features")) {
            item.children[0].value = "Added";
            changed = true;
          }
        }
        if (
          item.children.length !== 1 ||
          item.children[0].type !== "text" ||
          ![
            "Added",
            "Changed",
            "Deprecated",
            "Removed",
            "Fixed",
            "Security",
          ].includes(item.children[0].value)
        ) {
          console.log(
            `${path}:${item.children[0].position.start.line}:${item.children[0].position.start.column}`
          );
          return {
            correct: false,
            reason: `Level 3 headings should only be one of 'Added', 'Changed', 'Deprecated', 'Removed', 'Fixed', 'Security'`,
          };
        }

        // check that there is something other than a heading following it, which we have to presume describes the change
        if (!content[i + 1] || content[i + 1].type === "heading") {
          console.log(
            `${path}:${item.position.start.line}:${item.position.start.column}`
          );
          return {
            correct: false,
            reason:
              "Level 3 headings must be followed by something other than the next heading - you should use text to describe the change",
          };
        }
      }
    }

    if (changed) {
      // ...work around convoluted API...
      const wat = { data() {} };
      remarkStringify.call(wat);
      const output = wat.compiler(ast);
      writeFileSync(path, `# Changelog\n\n${output}`, "utf8");
    }

    return { correct: true, changed };
  } catch (err) {
    console.error("Must be an error in my checks: ", err);
    process.exit(1);
  }
}
