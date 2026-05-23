use serde::{Deserialize, Serialize};

pub mod parser;

unsafe extern "C" {
    fn new_parser() -> parser::TParser;
    fn init_parser(pst: *mut parser::TParser, text: *mut u32, len: i32);
    fn get_token(pst: *mut parser::TParser) -> i32;
}

pub fn setlocale() -> Result<(), std::io::Error> {
    unsafe {
        let locale_name = std::ffi::CString::new("")?;
        libc::setlocale(libc::LC_ALL, locale_name.as_ptr());
    }
    Ok(())
}

/// a token
#[derive(Debug, Serialize, Deserialize)]
pub struct Token<'a> {
    pub text: &'a str,
    pub kind: i32,
    pub char_index: usize,
    pub char_length: usize,
    pub byte_index: usize,
    pub byte_length: usize,
    pub line_number: usize,
}

impl<'a> Token<'a> {
    pub fn is_word(&self) -> bool {
        matches!(
            self.kind,
            parser::TS_WORD | parser::TS_ABBREV | parser::TS_COMPOUND
        )
    }
}

pub fn tokenize<'a>(s: &'a str) -> Vec<Token<'a>> {
    let mut tokens = Vec::new();
    unsafe {
        // build the C string
        let ws = &mut widestring::U32String::from_str(&s);
        let wptr = ws.as_mut_ustr();
        let ptr = wptr.as_mut_ptr();
        let ws_len = ws.len() as i32;
        // make the parser from it
        let mut pst = new_parser();
        init_parser(&mut pst, ptr, ws_len);
        // get tokens
        loop {
            let token_type = get_token(&mut pst);
            if token_type == parser::TS_END {
                break;
            } else {
                if token_type != parser::TS_SPACE {
                    let bidx = pst.token.byte_index as usize;
                    let blen = pst.token.byte_length as usize;
                    tokens.push(Token {
                        text: &s[bidx..bidx + blen],
                        kind: token_type,
                        byte_index: bidx,
                        byte_length: blen,
                        char_index: pst.token.index as usize,
                        char_length: pst.token.length as usize,
                        line_number: pst.token.line_number as usize,
                    });
                }
            }
        }
    }
    tokens
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_tok() {
        setlocale().unwrap();
        let text = "Oùùù puis-je m'installer?";
        let tokens = tokenize(text);
        let t = &tokens[2];
        assert_eq!(&text[t.byte_index..t.byte_index + t.byte_length], "-je");
        let t = &tokens[0];
        assert_eq!(&text[t.byte_index..t.byte_index + t.byte_length], "Oùùù");
        assert_eq!(t.text, "Oùùù");
        let _ = text
            .to_owned()
            .insert_str(t.byte_index + t.byte_length, "__");
        let _ = text.to_owned().insert_str(t.byte_index, "__");
    }

    #[test]
    fn test_ttype() {
        setlocale().unwrap();
        let text = "puis-je?";
        let tokens = tokenize(text);
        let verb = &tokens[0];
        let subj = &tokens[1];
        assert_eq!(verb.kind, subj.kind);
    }
}
