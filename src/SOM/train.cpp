
int update(vec,alpha,diam,code)
    Ds vec;
    double alpha;
    double diam;
    code_p code;
{
    int close_x, close_y, x, y, i;
    (void)find_closest_code(vec,&close_x,&close_y,code);
    for ( x=0; x<code->X; x++ ) {
        for ( y=0; y<code->Y; y++ ) {
            double p = prox(close_x,close_y,x,y,diam);
            if ( p < 0.0001 ) continue;
            for ( i=0; i<code->wid; i++ ) {
                code->val[x][y][i] +=
                              alpha * p * ( vec[i] - code->val[x][y][i] );
                }
            }
        }
    return 1;
}

int train_code(rlen,alpha0,diam0,K,data,code)
    long rlen;
    double alpha0;
    double diam0;
    double K;
    data_p data;
    code_p code;
{
    long to_do = rlen;
    int i;
    long t = 0;
    double diam  = diam0;
    double alpha = alpha0;
    double lambda = exp(-K);
    if ( data->wid != code->wid ) return -1;
    if ( rlen <= 0 ) return 0;
    for ( i=0; i<data->len && to_do>=0; i++, to_do-- ) {
        update( data->val[i], alpha, diam, code );
        }
    init_index(data->len);
    for ( t=0; t<to_do; t++, i++ ) {
        int index = next_index(data->len);
        if ( i >= data->len ) {
            i = 0;
            diam  = diam * lambda;
            alpha = alpha * lambda;
            }
        update( data->val[index], alpha, diam, code );
        }
    return 1;
}

