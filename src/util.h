static void free_buf(char *data, void *hint) {
  //fprintf(stderr, "Free %p\n", data);
  free(data);
}

static void dbt_set(DBT *dbt, void *data, u_int32_t size, u_int32_t flags = DB_DBT_USERMEM) {
  memset(dbt, 0, sizeof(*dbt));
  dbt->data = data;
  dbt->size = size;
  dbt->flags = flags;
}
