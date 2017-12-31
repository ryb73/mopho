let config = DbConfig.config;

let connOpts =
  Mysql.Connection.options(
    ~host=config.host,
    ~user=config.user,
    ~password=config.password,
    ~database=config.database,
    ()
  );

let opts = Mysql.Pool.options(connOpts);

let pool = Mysql.Pool.make(opts);
