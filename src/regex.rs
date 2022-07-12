use std::io::{self, Write};
use tree_sitter::{Parser, Query, QueryCursor};

pub struct RegexLiteral<'a> {
    pub pattern: &'a str,
    pub flags: &'a str,
}

/// Given some source javascript text, find regular expression literals.
pub fn find_literals(text: &str) -> Vec<RegexLiteral> {
    let mut regex_literals = Vec::new();

    let mut parser = Parser::new();
    parser
        .set_language(tree_sitter_javascript::language())
        .expect("Error loading the tree-sitter javascript grammar!");

    let parsed = parser.parse(text, None).unwrap();

    let regex_query = Query::new(
        tree_sitter_javascript::language(),
        "(regex pattern: (regex_pattern) @pattern flags: (regex_flags)? @flags)",
    )
    .expect("Failed to compile regex query");

    let mut cursor = QueryCursor::new();
    let source = text.as_bytes();
    for m in cursor.matches(&regex_query, parsed.root_node(), source) {
        let mut lit = RegexLiteral {
            pattern: m.captures[0].node.utf8_text(source).unwrap(),
            flags: "",
        };

        if m.captures.len() == 2 {
            lit.flags = m.captures[1].node.utf8_text(source).unwrap();
        }

        regex_literals.push(lit);
    }

    regex_literals
}

const PREAMBLE: &str = ";{
// Precompiled regular expressions
const precompile = (r) => { r.exec('a'); r.exec('\\u1000'); }";

/// Emit a block of javascript that will pre-compile the regular expressions given. As spidermonkey
/// will intern regular expressions, duplicating them at the top level and testing them with both
/// an ascii and utf8 string should ensure that they won't be re-compiled when run in the fetch
/// handler.
pub fn precompile<Out: Write>(literals: &[RegexLiteral], out: &mut Out) -> io::Result<()> {
    writeln!(out, "{}", PREAMBLE)?;

    for r in literals {
        writeln!(out, "precompile(/{}/{});", r.pattern, r.flags)?;
    }

    writeln!(out, "}}")?;
    Ok(())
}
