CREATE temp TABLE sentences (
    sent text
);

\copy sentences from 'tests/sentences.csv'

SELECT
    array_agg(x.token)
FROM
    sentences s,
    ts_debug('jusquci', s.sent) AS x
where x.alias != 'space'
group by s;
