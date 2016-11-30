<?php

/*
DEFAULT SETTINGS OF THE FLAT_DEPOSIT MODULE AND FUNCTIONS TO SET THESE
*/

function flat_deposit_set_paths($updated_values=NULL)
{

    $defaults =  array(
        #backend directory for freeze of data
        'freeze' => '/app/flat/backend/',
        #backend directory where bags will be placed
        'bag'=> "/app/flat/deposit/bags/",
        # directory sword uses to temporarily save bags
        'sword_tmp_dir' => '/app/flat/deposit/sword/tmp/',
        # root of the alternative directory where ingest service can search for archiving files.
        # This root need to contain user data folders named just as the LDAP login name
        'alternate_dir' => '/var/www/owncloud/data/',
        # (optional) subdirectory following user login name (e.g. in owncloud this is '/files')
        'alternate_subdir' => '/files'
    );

    //remove already installed variables
    $values_stored_in_database = variable_get('flat_deposit_paths', array());

    if (!empty($values_stored_in_database[0])) {variable_del('flat_deposit_paths'); }


    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_paths', $values );

    return TRUE;
}

function flat_deposit_set_names($updated_values=NULL)
{
    $defaults =   array(
        'apache_user' => 'www-data'
    );

    $values_stored_in_database = variable_get('flat_deposit_names', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {variable_del('flat_deposit_names'); }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_names', $values );


    return TRUE;
}

function flat_deposit_set_metadata($updated_values=NULL)
{
    $defaults = array(
        'site' => 'MPI',
        'type' => 'cmdi',
        'prefix' => 'lat',
        'MdProfile' => 'clarin.eu:cr1:p_1407745712035',

    );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_deposit_metadata', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {variable_del('flat_deposit_metadata'); }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_metadata', $values );

    return TRUE;
}

function flat_deposit_set_ingest_service($updated_values=NULL)
{
    $defaults = array(
        # host name of http server
        'host_name' => '172.17.0.1',
        # the bagit executable
        'bag_exe' => '/usr/local/bin/bagit-4.9.0/bin/bag',
        # Should errors be logged
        'log_errors' => TRUE,
        # directory to log errors)
        'error_log_file' => '/app/flat/backend/Ingest_service_error.log',
        # Should everything be logged
        'log_all' => TRUE,
        # directory to log errors)
        'log_file_dir' => '/app/flat/backend/',
    );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_deposit_ingest_service', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {variable_del('flat_deposit_ingest_service'); }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_ingest_service', $values );
    return TRUE;
}





function flat_deposit_set_sword($updated_values=NULL)
{
    $defaults = array(
        'url' => 'https://localhost/easy-deposit/',
        'port' => 8443,
        'user' => 'flat',
        'password' => 'sword',

    );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_deposit_sword', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {variable_del('flat_deposit_sword'); }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_sword', $values );

    return TRUE;

}


function flat_deposit_set_doorkeeper($updated_values=NULL)
{
    $defaults =  array(
        'url' => "http://localhost/flat/doorkeeper/",
        'port' => 8080
    );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_deposit_doorkeeper', array());

//already installed
    if (!empty($values_stored_in_database[0])) {variable_del('flat_deposit_doorkeeper'); }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_doorkeeper', $values );

    return TRUE;
}


function flat_deposit_set_fedora($updated_values=NULL) {

    $defaults =  array(
        'host_name' => 'localhost',
        'host_ip' => '127.0.0.1',
        'port' => 8443,
        'scheme' => 'https',
        'user' => 'fedoraAdmin',
        'password' => 'fedora',
        'context' => 'fedora',
    );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_deposit_fedora', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {variable_del('flat_deposit_fedora'); }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_fedora', $values );

    return TRUE;
}



function flat_deposit_set_solr($updated_values=NULL)
{
    $defaults =  array(
        'schema' => 'https',
        'host_name' => 'lux125.mpi.nl',
        'port' => 8080,
        'path' => 'solr',
        'core' => 'collection1',
    );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_deposit_solr', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {variable_del('flat_deposit_solr'); }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_solr', $values );


    return TRUE;
}
