// Copyright 2010-2014 RethinkDB, all rights reserved.
#ifndef CLUSTERING_ADMINISTRATION_TABLES_TABLE_STATUS_HPP_
#define CLUSTERING_ADMINISTRATION_TABLES_TABLE_STATUS_HPP_

#include <string>
#include <vector>

#include "errors.hpp"
#include <boost/shared_ptr.hpp>

#include "clustering/administration/database_metadata.hpp"
#include "clustering/administration/namespace_metadata.hpp"
#include "clustering/administration/tables/table_common.hpp"
#include "concurrency/watchable.hpp"

class server_name_client_t;

// Used to turn the directory watchable into a watchable for just one table
class table_directory_converter_t :
    public watchable_map_transform_t<std::pair<peer_id_t, namespace_id_t>,
                                     namespace_directory_metadata_t,
                                     peer_id_t,
                                     namespace_directory_metadata_t>
{
public:
    table_directory_converter_t(watchable_map_t<std::pair<peer_id_t, namespace_id_t>,
                                                namespace_directory_metadata_t> *_directory,
                                namespace_id_t _table_id);
    bool key_1_to_2(const std::pair<peer_id_t, namespace_id_t> &key1,
                    peer_id_t *key2_out);
    void value_1_to_2(const namespace_directory_metadata_t *value1,
                      const namespace_directory_metadata_t **value2_out);
    bool key_2_to_1(const peer_id_t &key2,
                    std::pair<peer_id_t, namespace_id_t> *key1_out);
    namespace_id_t table_id;
};

class table_status_artificial_table_backend_t :
    public common_table_artificial_table_backend_t
{
public:
    table_status_artificial_table_backend_t(
            boost::shared_ptr< semilattice_readwrite_view_t<
                cow_ptr_t<namespaces_semilattice_metadata_t> > > _table_sl_view,
            boost::shared_ptr< semilattice_readwrite_view_t<
                databases_semilattice_metadata_t> > _database_sl_view,
            watchable_map_t<std::pair<peer_id_t, namespace_id_t>,
                namespace_directory_metadata_t> *_directory_view,
            server_name_client_t *_name_client) :
        common_table_artificial_table_backend_t(_table_sl_view, _database_sl_view),
        directory_view(_directory_view),
        name_client(_name_client) { }

    bool write_row(
            ql::datum_t primary_key,
            bool pkey_was_autogenerated,
            ql::datum_t new_value,
            signal_t *interruptor,
            std::string *error_out);

    boost::optional<table_readiness_t> get_table_readiness(const namespace_id_t &table_id) const;

private:
    bool format_row(
            namespace_id_t table_id,
            name_string_t table_name,
            name_string_t db_name,
            const namespace_semilattice_metadata_t &metadata,
            signal_t *interruptor,
            ql::datum_t *row_out,
            std::string *error_out);

    watchable_map_t<std::pair<peer_id_t, namespace_id_t>,
        namespace_directory_metadata_t> *directory_view;
    server_name_client_t *name_client;
};

#endif /* CLUSTERING_ADMINISTRATION_TABLES_TABLE_STATUS_HPP_ */

