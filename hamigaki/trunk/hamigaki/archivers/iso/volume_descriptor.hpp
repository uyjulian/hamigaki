// volume_descriptor.hpp: ISO 9660 volume descriptor

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO_VOLUME_DESCRIPTOR_HPP
#define HAMIGAKI_ARCHIVERS_ISO_VOLUME_DESCRIPTOR_HPP

#include <hamigaki/archivers/iso/directory_record.hpp>
#include <boost/mpl/joint_view.hpp>
#include <cstring>

namespace hamigaki { namespace archivers { namespace iso {

struct volume_descriptor
{
    boost::uint8_t type;
    char std_id[5];
    boost::uint8_t version;
    boost::uint8_t flags;
    char system_id[32];
    char volume_id[32];
    boost::uint32_t volume_space_size;
    char escape_sequences[32];
    boost::uint16_t volume_set_size;
    boost::uint16_t volume_seq_number;
    boost::uint16_t logical_block_size;
    boost::uint32_t path_table_size;
    boost::uint32_t l_path_table_pos;
    boost::uint32_t l_path_table_pos2;
    boost::uint32_t m_path_table_pos;
    boost::uint32_t m_path_table_pos2;
    directory_record root_record;
    char root_file_id;
    char volume_set_id[128];
    char publisher_id[128];
    char data_preparer_id[128];
    char application_id[128];
    char copyright_file_id[37];
    char abstract_file_id[37];
    char bibliographic_file_id[37];
    date_time creation_time;
    date_time modification_time;
    date_time expiration_time;
    date_time effective_time;
    boost::uint8_t file_structure_version;
    char application_use[512];

    bool is_joliet() const
    {
        if (type != '\x02')
            return false;

        return
            (std::memcmp(escape_sequences, "%/@", 4) == 0) ||
            (std::memcmp(escape_sequences, "%/C", 4) == 0) ||
            (std::memcmp(escape_sequences, "%/E", 4) == 0) ;
    }
};

} } } // End namespaces iso, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::iso::volume_descriptor>
{
private:
    typedef archivers::iso::volume_descriptor self;
    typedef archivers::iso::directory_record directory_record;
    typedef archivers::iso::date_time date_time;

    typedef boost::mpl::list<
        member<self, boost::uint8_t, &self::type>,
        member<self, char[5], &self::std_id>,
        member<self, boost::uint8_t, &self::version>,
        member<self, boost::uint8_t, &self::flags>,
        member<self, char[32], &self::system_id>,
        member<self, char[32], &self::volume_id>,
        padding<8>,
        member<self, boost::uint32_t, &self::volume_space_size, little>,
        member<self, boost::uint32_t, &self::volume_space_size, big>,
        member<self, char[32], &self::escape_sequences>,
        member<self, boost::uint16_t, &self::volume_set_size, little>,
        member<self, boost::uint16_t, &self::volume_set_size, big>,
        member<self, boost::uint16_t, &self::volume_seq_number, little>,
        member<self, boost::uint16_t, &self::volume_seq_number, big>,
        member<self, boost::uint16_t, &self::logical_block_size, little>,
        member<self, boost::uint16_t, &self::logical_block_size, big>
    > members0;

    typedef boost::mpl::list<
        member<self, boost::uint32_t, &self::path_table_size, little>,
        member<self, boost::uint32_t, &self::path_table_size, big>,
        member<self, boost::uint32_t, &self::l_path_table_pos, little>,
        member<self, boost::uint32_t, &self::l_path_table_pos2, little>,
        member<self, boost::uint32_t, &self::m_path_table_pos, big>,
        member<self, boost::uint32_t, &self::m_path_table_pos2, big>,
        member<self, directory_record, &self::root_record>,
        member<self, char, &self::root_file_id>,
        member<self, char[128], &self::volume_set_id>,
        member<self, char[128], &self::publisher_id>,
        member<self, char[128], &self::data_preparer_id>,
        member<self, char[128], &self::application_id>,
        member<self, char[37], &self::copyright_file_id>,
        member<self, char[37], &self::abstract_file_id>,
        member<self, char[37], &self::bibliographic_file_id>
    > members1;

    typedef boost::mpl::list<
        member<self, date_time, &self::creation_time>,
        member<self, date_time, &self::modification_time>,
        member<self, date_time, &self::expiration_time>,
        member<self, date_time, &self::effective_time>,
        member<self, boost::uint8_t, &self::file_structure_version>,
        padding<1>,
        member<self, char[512], &self::application_use>,
        padding<653>
    > members2;

public:
    typedef boost::mpl::joint_view<
        members0,
        boost::mpl::joint_view<
            members1,
            members2
        >
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_ISO_VOLUME_DESCRIPTOR_HPP
