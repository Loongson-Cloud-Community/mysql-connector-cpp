/*
   Copyright (c) 2008, 2010, Oracle and/or its affiliates. All rights reserved.

   The MySQL Connector/C++ is licensed under the terms of the GPL
   <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
   MySQL Connectors. There are special exceptions to the terms and
   conditions of the GPL as it is applied to this software, see the
   FLOSS License Exception
   <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
*/


#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>

#include <cppconn/exception.h>
#include "mysql_util.h"
#include "mysql_ps_resultset.h"
#include "mysql_ps_resultset_metadata.h"

#include "nativeapi/native_statement_wrapper.h"
#include "nativeapi/native_resultset_wrapper.h"

#include "mysql_debug.h"

#define MAX_LEN_PER_CHAR 4

namespace sql
{
namespace mysql
{


/* {{{ MySQL_PreparedResultSetMetaData::MySQL_PreparedResultSetMetaData -I- */
MySQL_PreparedResultSetMetaData::MySQL_PreparedResultSetMetaData(boost::shared_ptr< NativeAPI::NativeStatementWrapper > & _proxy,
																	boost::shared_ptr< MySQL_DebugLogger> & l)
	: proxy(_proxy), logger(l), result_meta( _proxy->result_metadata()),
		num_fields(_proxy->field_count())
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::MySQL_PreparedResultSetMetaData");
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::~MySQL_PreparedResultSetMetaData -I- */
MySQL_PreparedResultSetMetaData::~MySQL_PreparedResultSetMetaData()
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::~MySQL_PreparedResultSetMetaData");
	CPP_INFO_FMT("this=%p", this);
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::checkColumnIndex -I- */
void
MySQL_PreparedResultSetMetaData::checkColumnIndex(unsigned int columnIndex) const
{
	if (columnIndex == 0 || columnIndex > num_fields) {
		throw sql::InvalidArgumentException("Invalid value for columnIndex");
	}
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::getCatalogName -I- */
SQLString
MySQL_PreparedResultSetMetaData::getCatalogName(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::getCatalogName");
	CPP_INFO_FMT("this=%p", this);
	checkColumnIndex(columnIndex);
	return getFieldMeta(columnIndex)->catalog;
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::getColumnCount -I- */
unsigned int
MySQL_PreparedResultSetMetaData::getColumnCount()
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::getColumnCount");
	CPP_INFO_FMT("this=%p", this);
	return num_fields;
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::getColumnDisplaySize -I- */
unsigned int
MySQL_PreparedResultSetMetaData::getColumnDisplaySize(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::getColumnDisplaySize");
	CPP_INFO_FMT("this=%p", this);
	checkColumnIndex(columnIndex);
	const MYSQL_FIELD * const field = getFieldMeta(columnIndex);
	const sql::mysql::util::OUR_CHARSET * const cs = sql::mysql::util::find_charset(field->charsetnr);
	if (!cs) {
		std::ostringstream msg("Server sent uknown charsetnr (");
		msg << field->charsetnr << ") . Please report";
		throw SQLException(msg.str());
	}
	int ret = field->length / cs->char_maxlen;
	CPP_INFO_FMT("column=%u display_size=%d", columnIndex, ret);
	return ret;
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::getColumnLabel -I- */
SQLString
MySQL_PreparedResultSetMetaData::getColumnLabel(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::getColumnLabel");
	CPP_INFO_FMT("this=%p", this);
	checkColumnIndex(columnIndex);
	return getFieldMeta(columnIndex)->name;
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::getColumnName -I- */
SQLString
MySQL_PreparedResultSetMetaData::getColumnName(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::getColumnName");
	CPP_INFO_FMT("this=%p", this);
	checkColumnIndex(columnIndex);
	return getFieldMeta(columnIndex)->name;
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::getColumnType -I- */
int
MySQL_PreparedResultSetMetaData::getColumnType(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::getColumnType");
	CPP_INFO_FMT("this=%p", this);
	CPP_INFO_FMT("column=%u", columnIndex);
	checkColumnIndex(columnIndex);
	int mysql_type = getFieldMeta(columnIndex)->type;
	CPP_INFO_FMT("type=%d", mysql_type);
	int ret = sql::mysql::util::mysql_type_to_datatype(
					getFieldMeta(columnIndex)
				);
	CPP_INFO_FMT("our type is %d", ret);
	return ret;
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::getColumnTypeName -I- */
SQLString
MySQL_PreparedResultSetMetaData::getColumnTypeName(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::getColumnTypeName");
	CPP_INFO_FMT("this=%p", this);
	checkColumnIndex(columnIndex);
	return sql::mysql::util::mysql_type_to_string(
				getFieldMeta(columnIndex)
			);
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::getFieldMeta -I- */
MYSQL_FIELD * MySQL_PreparedResultSetMetaData::getFieldMeta(unsigned int columnIndex) const
{
	return result_meta->fetch_field_direct(columnIndex - 1);
}
/* }}} */


// Precision - total number of digits
/* {{{ MySQL_PreparedResultSetMetaData::getPrecision -I- */
unsigned int
MySQL_PreparedResultSetMetaData::getPrecision(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::getPrecision");
	CPP_INFO_FMT("this=%p", this);
	checkColumnIndex(columnIndex);
	unsigned int ret = getFieldMeta(columnIndex)->max_length - getScale(columnIndex);
	CPP_INFO_FMT("column=%u precision=%d", columnIndex, ret);
	return ret;
}
/* }}} */


// Scale - Number of digits right of the decimal point
/* {{{ MySQL_PreparedResultSetMetaData::getScale -I- */
unsigned int
MySQL_PreparedResultSetMetaData::getScale(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::getScale");
	CPP_INFO_FMT("this=%p", this);
	checkColumnIndex(columnIndex);
	unsigned int ret = getFieldMeta(columnIndex)->decimals;
	CPP_INFO_FMT("column=%u scale=%d", columnIndex, ret);
	return ret;
}
/* }}} */



/* {{{ MySQL_PreparedResultSetMetaData::getSchemaName -I- */
SQLString
MySQL_PreparedResultSetMetaData::getSchemaName(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::getSchemaName");
	CPP_INFO_FMT("this=%p", this);
	checkColumnIndex(columnIndex);
	const char * const db = getFieldMeta(columnIndex)->db;
	return db ? db : "";
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::getTableName -I- */
SQLString
MySQL_PreparedResultSetMetaData::getTableName(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::getTableName");
	CPP_INFO_FMT("this=%p", this);
	checkColumnIndex(columnIndex);
	return getFieldMeta(columnIndex)->org_table;
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::isAutoIncrement -I- */
bool
MySQL_PreparedResultSetMetaData::isAutoIncrement(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::isAutoIncrement");
	CPP_INFO_FMT("this=%p", this);
	checkColumnIndex(columnIndex);
	return (getFieldMeta(columnIndex)->flags & AUTO_INCREMENT_FLAG ) != 0;
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::isCaseSensitive -I- */
bool
MySQL_PreparedResultSetMetaData::isCaseSensitive(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::isCaseSensitive");
	CPP_INFO_FMT("this=%p", this);
	checkColumnIndex(columnIndex);
	const MYSQL_FIELD * const field = getFieldMeta(columnIndex);
	if (field->flags & NUM_FLAG || field->type == MYSQL_TYPE_NEWDECIMAL || field->type == MYSQL_TYPE_DECIMAL) {
		return false;
	}
	const sql::mysql::util::OUR_CHARSET * const cs = sql::mysql::util::find_charset(field->charsetnr);
	if (!cs) {
		std::ostringstream msg("Server sent uknown charsetnr (");
		msg << field->charsetnr << ") . Please report";
		throw SQLException(msg.str());
	}
	return NULL == strstr(cs->collation, "_ci");
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::isCurrency -I- */
bool
MySQL_PreparedResultSetMetaData::isCurrency(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::isCurrency");
	CPP_INFO_FMT("this=%p", this);
	checkColumnIndex(columnIndex);
	return false;
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::isDefinitelyWritable -I- */
bool
MySQL_PreparedResultSetMetaData::isDefinitelyWritable(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::isDefinitelyWritable");
	CPP_INFO_FMT("this=%p", this);
	checkColumnIndex(columnIndex);
	return isWritable(columnIndex);
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::isNullable -I- */
int
MySQL_PreparedResultSetMetaData::isNullable(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::isNullable");
	CPP_INFO_FMT("this=%p", this);
	checkColumnIndex(columnIndex);
	return getFieldMeta(columnIndex)->flags & NOT_NULL_FLAG? columnNoNulls : columnNullable;
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::isReadOnly -I- */
bool
MySQL_PreparedResultSetMetaData::isReadOnly(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::isReadOnly");
	CPP_INFO_FMT("this=%p", this);
	checkColumnIndex(columnIndex);
	/* Seems for Views, where the value is generated DB is empty everything else is set */
	const char * const db = getFieldMeta(columnIndex)->db;
	return !(db && strlen(db));
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::isSearchable -I- */
bool
MySQL_PreparedResultSetMetaData::isSearchable(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::isSearchable");
	CPP_INFO_FMT("this=%p", this);
	checkColumnIndex(columnIndex);
	return true;
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::isSigned -I- */
bool
MySQL_PreparedResultSetMetaData::isSigned(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::isSigned");
	CPP_INFO_FMT("this=%p", this);
	checkColumnIndex(columnIndex);
	if (getFieldMeta(columnIndex)->type == FIELD_TYPE_YEAR) {
		return false;
	}
	return !(getFieldMeta(columnIndex)->flags & UNSIGNED_FLAG);
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::isWritable -I- */
bool
MySQL_PreparedResultSetMetaData::isWritable(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::isWritable");
	CPP_INFO_FMT("this=%p", this);
	checkColumnIndex(columnIndex);
	return !isReadOnly(columnIndex);
}
/* }}} */


/* {{{ MySQL_PreparedResultSetMetaData::isZerofill -I- */
bool
MySQL_PreparedResultSetMetaData::isZerofill(unsigned int columnIndex)
{
	CPP_ENTER("MySQL_PreparedResultSetMetaData::isZerofill");
	CPP_INFO_FMT("this=%p", this);
	checkColumnIndex(columnIndex);
	return (getFieldMeta(columnIndex)->flags & ZEROFILL_FLAG) != 0;
}
/* }}} */


} /* namespace mysql */
} /* namespace sql */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
