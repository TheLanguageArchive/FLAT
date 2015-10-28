FROM        phusion/baseimage
MAINTAINER  Menzo Windhouwer <menzo.windhouwer@meertens.knaw.nl>

# Set correct environment variables.
ENV HOME /root

# Regenerate SSH host keys. baseimage-docker does not contain any, so you
# have to do that yourself. You may also comment out this instruction; the
# init system will auto-generate one during boot.
RUN /etc/my_init.d/00_regen_ssh_host_keys.sh

# Use baseimage-docker's init system.
CMD ["/sbin/my_init"]

#
# Update and install all system libraries
#
RUN apt-get update &&\
	apt-get -y dist-upgrade &&\
	apt-get -y install curl wget openssl apache2 postgresql php5 php5-gd php5-pgsql php5-xsl php5-curl unzip default-jdk libapache2-mod-jk expect ssh rsync git maven

#
# Configure apache
#   - set apache ulimit to 1024
ADD apache/envvars /etc/apache2/envvars
ADD apache/apache2.conf /etc/apache2/apache2.conf
RUN cd /etc/apache2/mods-enabled &&\
	ln -s ../mods-available/rewrite.load . &&\
	ln -s ../mods-available/proxy.conf . &&\
	ln -s ../mods-available/proxy.load . &&\
	ln -s ../mods-available/proxy_http.load .

#
# Configure postgres
#   - update postgresql.conf to bind to localhost
#   - create database user and update pg_hba_conf for this user
#   - initialise databases for fedora and drupal
#
# TODO: fix snakeoil key permission issue without copying
RUN cp /etc/ssl/certs/ssl-cert-snakeoil.pem /etc/postgresql/ssl-cert-snakeoil.pem &&\
	chown postgres:postgres /etc/postgresql/ssl-cert-snakeoil.pem
RUN cp /etc/ssl/private/ssl-cert-snakeoil.key /etc/postgresql/ssl-cert-snakeoil.key &&\
	chown postgres:postgres /etc/postgresql/ssl-cert-snakeoil.key
ADD postgresql/pg_hba.conf /etc/postgresql/9.3/main/pg_hba.conf
ADD postgresql/postgresql.conf /etc/postgresql/9.3/main/postgresql.conf
RUN useradd -g postgres ssl-cert
RUN /etc/init.d/postgresql start &&\
	su -c "psql -c \"CREATE USER fedora WITH PASSWORD 'fedora'\"" postgres &&\
	su -c "createdb -O fedora fedora" postgres &&\
	su -c "createdb -O fedora proai" postgres &&\
	su -c "createdb -O fedora --encoding=UNICODE -T template0 drupal" postgres &&\
	/etc/init.d/postgresql stop

#
# Download packages for the islandora setup
#   - fedora commons
#   - proai
#   - drupal
#       - tuque
#   - islandora
#

ENV FEDORA_HOME /var/www/fedora
ENV CATALINA_HOME /var/www/fedora/tomcat

RUN mkdir /tmp/fedora
RUN cd /tmp/fedora &&\ 
	wget "http://downloads.sourceforge.net/project/fedora-commons/fedora/3.8.0/fcrepo-installer-3.8.0.jar"

ADD fedora/install.properties /tmp/fedora/install.properties
RUN /etc/init.d/postgresql start &&\
	cd /tmp/fedora &&\
	java -jar fcrepo-installer-3.8.0.jar install.properties &&\
	/etc/init.d/postgresql stop

#
# starting fedora will initialize required directories
#

ADD fedora/wait.sh /tmp/wait.sh
RUN chmod 0755 /tmp/wait.sh
RUN /etc/init.d/postgresql start &&\
	/var/www/fedora/tomcat/bin/startup.sh &&\
	/tmp/wait.sh &&\
	bash -c "timeout 60 grep -q '(AuthFilterJAAS) initialised servlet filter: org.fcrepo.server.security.jaas.AuthFilterJAAS' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
    # apparantly the shutdown port is not directly available after startup
 	sleep 10 &&\
	/var/www/fedora/tomcat/bin/shutdown.sh &&\
	bash -c "timeout 60 grep -q 'Server shutdown complete' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
	/etc/init.d/postgresql stop
RUN rm -v /var/www/fedora/data/fedora-xacml-policies/repository-policies/default/deny-purge-*
ADD fedora/permit-anything-to-localhost.xml /var/www/fedora/data/fedora-xacml-policies/repository-policies/default/permit-anything-to-localhost.xml
ADD fedora/deny-unallowed-file-resolution.xml /var/www/fedora/data/fedora-xacml-policies/repository-policies/default/deny-unallowed-file-resolution.xml
RUN cd /var/www/fedora/data/fedora-xacml-policies/repository-policies/ &&\
	wget https://github.com/Islandora/islandora-xacml-policies/archive/master.zip &&\
	unzip master.zip &&\
	rm master.zip &&\
	mv islandora-xacml-policies-master islandora
RUN cd /var/www/fedora/tomcat/webapps/fedora/WEB-INF/lib &&\ 
	wget "https://github.com/Islandora/islandora_drupal_filter/releases/download/v7.1.3/fcrepo-drupalauthfilter-3.8.0.jar"
ADD fedora/jaas.conf /var/www/fedora/server/config/jaas.conf
ADD fedora/filter-drupal.xml /var/www/fedora/server/config/filter-drupal.xml

##Download and configure drupal (via drush)
RUN mkdir -p /var/www/composer
RUN cd /var/www/composer &&\
	curl -sS https://getcomposer.org/installer | php &&\
	mv composer.phar composer
ENV COMPOSER_HOME=/var/www/composer
RUN $COMPOSER_HOME/composer global require drush/drush:6.*

RUN cd /var/www/html &&\
	/var/www/composer/vendor/drush/drush/drush dl drupal --drupal-project-rename=drupal
ENV PGPASSWORD=fedora
RUN /etc/init.d/postgresql start &&\
	cd /var/www/html/drupal &&\
	export PHP_OPTIONS="-d sendmail_path=`which true`" &&\
	/var/www/composer/vendor/drush/drush/drush site-install standard --site-name=FLAT --account-name=admin --account-pass=admin --db-su=fedora --db-su-pw=fedora --db-url=pgsql://fedora:fedora@localhost/drupal -y &&\
	/etc/init.d/postgresql stop

RUN mkdir -p /var/www/html/drupal/sites/all/modules/custom
RUN chown www-data:www-data /var/www/html/drupal/sites/default/files &&\
	chown www-data:www-data /var/www/html/drupal/sites/default/settings.php &&\
	chown www-data:www-data /var/www/html/drupal/sites/all/modules/custom
    
ADD drupal/flat-logo.png /var/www/html/drupal/sites/default/files/flat-logo.png
ADD drupal/flat /var/www/html/drupal/sites/all/modules/custom/
ADD drupal/flat-logo.php /tmp/

RUN mkdir /tmp/tuque &&\
	cd /tmp/tuque &&\
	wget "https://github.com/islandora/tuque/archive/1.4.zip" &&\
	unzip 1.4.zip &&\
	mv /tmp/tuque/tuque-1.4 /var/www/html/drupal/sites/all/libraries/tuque
RUN sed -i 's|public $verifyPeer = TRUE;|public $verifyPeer = FALSE;|g' /var/www/html/drupal/sites/all/libraries/tuque/HttpConnection.php

RUN mkdir -p /tmp/islandora
RUN mkdir -p /var/www/html/drupal/sites/all/modules/contrib
RUN mkdir -p /var/www/html/drupal/sites/all/themes
RUN cd /tmp/islandora &&\
	wget "https://github.com/Islandora/islandora/archive/7.x-1.5.tar.gz" &&\
	tar -xf 7.x-1.5.tar.gz -C /var/www/html/drupal/sites/all/modules/contrib/
RUN cd /tmp/islandora &&\
	wget "https://github.com/islandora/islandora_solution_pack_collection/archive/7.x-1.5.zip" &&\
	mv 7.x-1.5.zip islandora_solution_pack_collection-7.x-1.5.zip &&\
	unzip islandora_solution_pack_collection-7.x-1.5.zip -d /var/www/html/drupal/sites/all/modules/contrib/
RUN cd /tmp/islandora &&\
	wget "https://github.com/Islandora/islandora_solution_pack_compound/archive/7.x-1.5.zip" &&\
	mv 7.x-1.5.zip islandora_solution_pack_compound-7.x-1.5.zip &&\
	unzip islandora_solution_pack_compound-7.x-1.5.zip -d /var/www/html/drupal/sites/all/modules/contrib/

RUN mkdir -p /var/www/html/drupal/sites/all/modules/contrib/islandora_solution_pack_cmdi
ADD flat/islandora_solution_pack_cmdi /var/www/html/drupal/sites/all/modules/contrib/islandora_solution_pack_cmdi
    
RUN mkdir -p /tmp/bootstrap
RUN cd /tmp/bootstrap &&\
	wget "http://ftp.drupal.org/files/projects/bootstrap-7.x-3.x-dev.zip" &&\
	unzip bootstrap-7.x-3.x-dev.zip &&\
	mv /tmp/bootstrap/bootstrap /var/www/html/drupal/sites/all/themes/bootstrap

RUN cp -r /var/www/html/drupal/sites/all/themes/bootstrap/starterkits/cdn  /var/www/html/drupal/sites/all/themes/flat_bootstrap_theme &&\
	cd /var/www/html/drupal/sites/all/themes/flat_bootstrap_theme/ &&\
	mv cdn.starterkit flat_bootstrap_theme.info &&\
	echo "settings[bootstrap_cdn_jsdelivr_theme] = 'flatly'" >> flat_bootstrap_theme.info

RUN cd /var/www/html/drupal/ &&\
	export PGPASSWORD=fedora &&\
	/etc/init.d/postgresql start &&\
	/var/www/fedora/tomcat/bin/startup.sh &&\
	bash -c "timeout 60 grep -q '(AuthFilterJAAS) initialised servlet filter: org.fcrepo.server.security.jaas.AuthFilterJAAS' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
	/var/www/composer/vendor/drush/drush/drush vset islandora_base_url https://localhost:8443/fedora &&\
	/var/www/composer/vendor/drush/drush/drush en islandora -y &&\
	/var/www/composer/vendor/drush/drush/drush en islandora_basic_collection -y &&\
	/var/www/composer/vendor/drush/drush/drush en islandora_compound_object -y &&\
	/var/www/composer/vendor/drush/drush/drush en islandora_cmdi -y &&\
	/var/www/composer/vendor/drush/drush/drush vset islandora_metadata_display cmdi &&\
	/var/www/composer/vendor/drush/drush/drush dl features_extra -y &&\
	/var/www/composer/vendor/drush/drush/drush en flat -y &&\
	/var/www/composer/vendor/drush/drush/drush php-script flat-logo --script-path=/tmp &&\
	/var/www/composer/vendor/drush/drush/drush en jquery_update -y &&\
	/var/www/composer/vendor/drush/drush/drush en bootstrap -y &&\
	/var/www/composer/vendor/drush/drush/drush en flat_bootstrap_theme -y &&\
	/var/www/composer/vendor/drush/drush/drush vset theme_default flat_bootstrap_theme &&\
	/var/www/fedora/tomcat/bin/shutdown.sh &&\
	bash -c "timeout 60 grep -q 'Server shutdown complete' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
	/etc/init.d/postgresql stop
    
# add the generated certificate for localhost
ADD cert/InstallCert.jar /tmp/InstallCert.jar
ADD cert/InstallCert.sh /tmp/InstallCert.sh
RUN chmod +x /tmp/InstallCert.sh
RUN cd /opt &&\
	/etc/init.d/postgresql start &&\
	/var/www/fedora/tomcat/bin/startup.sh &&\
	bash -c "timeout 60 grep -q '(AuthFilterJAAS) initialised servlet filter: org.fcrepo.server.security.jaas.AuthFilterJAAS' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
	/tmp/InstallCert.sh &&\
	/var/www/fedora/tomcat/bin/shutdown.sh &&\
	bash -c "timeout 60 grep -q 'Server shutdown complete' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
	/etc/init.d/postgresql stop

##Download proai
RUN mkdir /tmp/proai &&\
	cd /tmp/proai &&\
	wget http://downloads.sourceforge.net/project/fedora-commons/services/3.4/oaiprovider-1.2.2.zip &&\
	unzip oaiprovider-1.2.2.zip &&\
	cp oaiprovider-1.2.2/oaiprovider.war /var/www/fedora/tomcat/webapps/oaiprovider.war &&\
	mkdir -p /var/www/fedora/tomcat/webapps/oaiprovider &&\
	cd /var/www/fedora/tomcat/webapps/oaiprovider &&\
	jar xf ../oaiprovider.war
ADD proai/proai.properties /var/www/fedora/tomcat/webapps/oaiprovider/WEB-INF/classes/proai.properties
ADD proai/identify.xml /var/www/html/identify.xml

#
# Add start script to automatically start required services
#

ENV JAVA_OPTS "-Djavax.net.ssl.trustStore=/opt/jssecacerts -Djavax.net.ssl.trustStorePassword=changeit"

ADD fedora/tomcat-fedora.sh /var/www/fedora/tomcat/bin/tomcat-fedora.sh
RUN chmod u+x /var/www/fedora/tomcat/bin/tomcat-fedora.sh

ADD my_init.d/start.sh /etc/my_init.d/start.sh
RUN chmod u+x /etc/my_init.d/start.sh

#
# Add FLAT's own scripts and tools 
#

RUN mkdir -p /app/flat
ADD flat/scripts/* /app/flat/
RUN chmod +x /app/flat/*.sh

RUN cd /tmp &&\
    git clone https://github.com/TheLanguageArchive/SchemAnon.git SchemAnon &&\
    cd /tmp/SchemAnon &&\
    mvn install:install-file -Dfile=lib/org/eclipse/wst/org.eclipse.wst.xml.xpath2.processor/1.1.0/org.eclipse.wst.xml.xpath2.processor-1.1.0.jar -DgroupId=org.eclipse.wst -DartifactId=org.eclipse.wst.xml.xpath2.processor -Dversion=1.1.0 -Dpackaging=jar &&\
    mvn install:install-file -Dfile=lib/xerces/xercesImpl/2.11.0-xml-schema-1.1-beta/xercesImpl-2.11.0-xml-schema-1.1-beta.jar -DpomFile=lib/xerces/xercesImpl/2.11.0-xml-schema-1.1-beta/xercesImpl-2.11.0-xml-schema-1.1-beta.pom -DgroupId=xerces -DartifactId=xercesImpl -Dversion=2.11.0-xml-schema-1.1-beta -Dpackaging=jar &&\
    mvn clean install

RUN cd /tmp &&\
    git clone -b development https://github.com/TheLanguageArchive/MetadataTranslator.git MetadataTranslator &&\
    cd /tmp/MetadataTranslator
    
RUN cd /tmp/MetadataTranslator &&\
    mvn clean install
    
RUN cd /tmp &&\
    wget https://nexus.clarin.eu/content/repositories/clarin-snapshot/eu/clarin/cmdi/cmdi-validator-core/0.0.6-SNAPSHOT/cmdi-validator-core-0.0.6-20150702.082849-1.jar &&\
    mvn install:install-file -Dfile=cmdi-validator-core-0.0.6-20150702.082849-1.jar -DgroupId=eu.clarin.cmdi -DartifactId=cmdi-validator-core -Dversion=0.0.6-SNAPSHOT -Dpackaging=jar
 
RUN cd /tmp &&\
    git clone https://github.com/TheLanguageArchive/ISLE2CLARIN.git ISLE2CLARIN &&\
    cd /tmp/ISLE2CLARIN &&\
    mvn clean package &&\
    mkdir -p /app/flat/lib &&\
    mv target/isle2clarin.jar /app/flat/lib/isle2clarin.jar

RUN mkdir -p /tmp/LAT2FOX
ADD flat/LAT2FOX /tmp/LAT2FOX
RUN cd /tmp/LAT2FOX &&\
    mvn clean package &&\
    mkdir -p /app/flat/lib &&\
    mv target/lat2fox.jar /app/flat/lib/lat2fox.jar
    
#Clean up APT and /tmp when done.
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

EXPOSE 80
EXPOSE 443
EXPOSE 8080
EXPOSE 8443