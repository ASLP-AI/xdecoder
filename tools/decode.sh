
tools/xdecode \
    --beam=13 --max-active=7000 \
    --acoustic-scale=0.0666667 --skip=0 --max-batch-size=256 \
    --num-bins=40 --left-context=5 --right-context=5 \
    --cmvn-file=config/cmvn \
    config/hclg config/tree config/am.net config/pdf_prior config/words.txt \
    config/wav.scp config/result.scp
