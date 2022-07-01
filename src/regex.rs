
/// Given some source javascript text, find regular expression literals with a heuristic, and
/// return them in a vector.
pub fn find_literals(mut text: &str) -> Vec<String> {

    let mut regex_literals = Vec::new();

    while let Some(mut pos) = text.find('/') {
        pos += 1;
        text = &text[pos..];

        if text.is_empty() {
            break
        }

        if text.chars().nth(0).unwrap() == '/' {
            text = &text[1..];
            continue
        }

        if let Some(end) = text.find('/') {
            if let Some(eol) = text.find('\n') {

                // regex literals don't span lines
                if eol < end {
                    text = &text[end..];
                    continue
                }
            }

            regex_literals.push(String::from(&text[0..end]));
            text = &text[end+1..]
        } else {
            break
        }
    }

    regex_literals
}
