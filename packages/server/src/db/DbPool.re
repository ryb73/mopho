let _config = DbConfig.config;

let _connOpts = Mysql.Connection.options(
    ~host=_config.host,
    ~user=_config.user,
    ~password=_config.password,
    ~database=_config.database, ()
);

let _opts = Mysql.Pool.options(_connOpts);
let pool = Mysql.Pool.make(_opts);
