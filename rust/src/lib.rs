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
pub struct Token {
    /// the token type
    pub ttype: i32,
    /// the token text
    pub ttext: String,
    /// the token char index
    pub cidx: usize,
    /// the number of chars in the token
    pub clen: usize,
    /// the token byte index
    pub bidx: usize,
    /// the number of bytes in the token
    pub blen: usize,
}

impl Token {
    pub fn is_word(&self) -> bool {
        matches!(
            self.ttype,
            parser::TS_WORD | parser::TS_ABBREV | parser::TS_COMPOUND
        )
    }
}

/// a parsed text
pub type Document = Vec<Token>;

pub fn tokenize(s: &str) -> Result<Vec<Token>, widestring::error::Utf32Error> {
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
        let mut bidx = 0;
        loop {
            let ttype = get_token(&mut pst);
            if ttype == parser::TS_END {
                break;
            } else {
                let cidx = pst.tidx as usize;
                let clen = pst.tlen as usize;
                let substr = &ws[cidx..cidx + clen];
                let ttext = substr.to_string()?;
                let blen = ttext.len();
                // skip spaces (but still increment byte index)
                if ttype != parser::TS_SPACE {
                    tokens.push(Token {
                        bidx,
                        blen,
                        ttype,
                        ttext,
                        cidx,
                        clen,
                    });
                }
                bidx += blen;
            }
        }
    }
    Ok(tokens)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_tok() {
        setlocale().unwrap();
        let text = "Oùùù puis-je m'installer?";
        let tokens = tokenize(text).unwrap();
        let t = &tokens[2];
        assert_eq!(&text[t.bidx..t.bidx + t.blen], "-je");
        let t = &tokens[0];
        assert_eq!(&text[t.bidx..t.bidx + t.blen], "Oùùù");
        assert_eq!(&t.ttext, "Oùùù");
        let _ = text.to_owned().insert_str(t.bidx+t.blen, "__");
        let _ = text.to_owned().insert_str(t.bidx, "__");
    }

    #[test]
    fn test_ttype() {
        setlocale().unwrap();
        let text = "puis-je?";
        let tokens = tokenize(text).unwrap();
        let verb = &tokens[0];
        let subj = &tokens[1];
        assert_eq!(verb.ttype, subj.ttype);
    }
}
